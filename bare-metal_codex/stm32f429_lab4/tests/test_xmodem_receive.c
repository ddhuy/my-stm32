#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lib/crc/crc16.h"
#include "lib/protocol/xmodem/xmodem.h"

/* Scripted input: byte values or -1 for one simulated timeout. No sleeping. */
#define STREAM_SIZE (260u * XMODEM_PACKET_SIZE)
#define BUFFER_SIZE (257u * XMODEM_BLOCK_SIZE)

static struct
{
    int input[STREAM_SIZE];
    size_t length, position, reads, writes;
    uint8_t output[1024];
} wire;

static uint8_t storage[BUFFER_SIZE + 2];
static size_t received;

static bool read_byte(void *context, uint8_t *byte, uint32_t timeout)
{
    assert(context == &wire);
    assert(timeout == XMODEM_START_TIMEOUT_MS || timeout == XMODEM_BYTE_TIMEOUT_MS);
    assert(++wire.reads < STREAM_SIZE + 1000u); /* Detect unbounded retry loops. */
    if (wire.position == wire.length)
        return false;
    int event = wire.input[wire.position++];
    if (event < 0)
        return false;
    *byte = (uint8_t)event;
    return true;
}

static void write_byte(void *context, uint8_t byte)
{
    assert(context == &wire);
    assert(wire.writes < sizeof wire.output);
    wire.output[wire.writes++] = byte;
}

static const xmodem_io_t io = {read_byte, write_byte, &wire};

static void reset(void)
{
    memset(&wire, 0, sizeof wire);
    memset(storage, 0xA5, sizeof storage);
    received = 999;
}

static void event(int value)
{
    assert(wire.length < STREAM_SIZE);
    wire.input[wire.length++] = value;
}

static void packet(uint8_t block, uint8_t fill, bool bad_crc)
{
    uint8_t data[XMODEM_BLOCK_SIZE];
    memset(data, fill, sizeof data);
    uint16_t crc = crc16_xmodem(data, sizeof data);
    if (bad_crc)
        crc ^= 1;
    event(XMODEM_SOH);
    event(block);
    event((uint8_t)~block);
    for (size_t i = 0; i < sizeof data; ++i)
        event(data[i]);
    event(crc >> 8);
    event(crc & 0xFF);
}

static void run(xmodem_status_t expected, size_t capacity, size_t size)
{
    assert(capacity <= BUFFER_SIZE);
    assert(xmodem_receive(&io, storage + 1, capacity, &received) == expected);
    assert(received == size);
    assert(storage[0] == 0xA5);
    for (size_t i = capacity + 1; i < sizeof storage; ++i)
        assert(storage[i] == 0xA5); /* No writes beyond the supplied capacity. */
    assert(wire.position == wire.length);
}

static void controls(const uint8_t *expected, size_t count)
{
    assert(wire.writes == count);
    assert(memcmp(wire.output, expected, count) == 0);
}

#define CONTROLS(...)                             \
    do                                            \
    {                                             \
        const uint8_t expected[] = {__VA_ARGS__}; \
        controls(expected, sizeof expected);      \
    } while (0)

static void filled(size_t offset, size_t count, uint8_t value)
{
    for (size_t i = 0; i < count; ++i)
        assert(storage[1 + offset + i] == value);
}

static void retry_controls(uint8_t retry)
{
    assert(wire.writes == XMODEM_MAX_RETRIES + 2);
    assert(wire.output[0] == XMODEM_CRC);
    for (size_t i = 1; i < XMODEM_MAX_RETRIES; ++i)
        assert(wire.output[i] == retry);
    assert(wire.output[wire.writes - 2] == XMODEM_CAN);
    assert(wire.output[wire.writes - 1] == XMODEM_CAN);
}

static void test_arguments(void)
{
    reset();
    xmodem_io_t invalid = io;
    assert(xmodem_receive(NULL, storage, 1, &received) == XMODEM_ERR_IO);
    invalid.read_byte = NULL;
    assert(xmodem_receive(&invalid, storage, 1, &received) == XMODEM_ERR_IO);
    invalid = io;
    invalid.write_byte = NULL;
    assert(xmodem_receive(&invalid, storage, 1, &received) == XMODEM_ERR_IO);
    assert(xmodem_receive(&io, NULL, 1, &received) == XMODEM_ERR_IO);
    assert(xmodem_receive(&io, storage, 1, NULL) == XMODEM_ERR_IO);
    assert(wire.reads == 0 && wire.writes == 0);
}

static void test_success(void)
{
    reset();
    packet(1, 0x1A, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 0x1A); /* Padding preserved. */
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK);
}

static void test_duplicate_block(void)
{
    /* Lost ACK: acknowledge block 1 again without advancing the destination. */
    reset();
    packet(1, 0x11, false);
    packet(1, 0x11, false);
    packet(2, 0x22, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 256, 256);
    filled(0, 128, 0x11);
    filled(128, 128, 0x22);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK, XMODEM_ACK, XMODEM_ACK);
    /* A duplicate must also be accepted when the buffer is already full. */
    reset();
    packet(1, 7, false);
    packet(1, 7, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 7);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK, XMODEM_ACK);

    /* A corrupt duplicate must be NAKed, then a valid retry ACKed. */
    reset();
    packet(1, 0x11, false);
    packet(1, 0x11, true);
    packet(1, 0x11, false);
    packet(2, 0x22, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 256, 256);
    filled(0, 128, 0x11);
    filled(128, 128, 0x22);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_NAK,
             XMODEM_ACK, XMODEM_ACK, XMODEM_ACK);

    /* Only the immediately preceding block qualifies as a duplicate. */
    reset();
    packet(1, 0x11, false);
    packet(2, 0x22, false);
    packet(1, 0x11, false);
    packet(3, 0x33, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 384, 384);
    filled(0, 128, 0x11);
    filled(128, 128, 0x22);
    filled(256, 128, 0x33);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK,
             XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);
}

static void test_crc_retransmission(void)
{
    /* Reject the first block's bad CRC, then accept its valid retransmission. */
    reset();
    packet(1, 3, true);
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 9);
    CONTROLS(XMODEM_CRC, XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);

    /* Repeated CRC failures must not advance the block number or overwrite
     * previously accepted data. Distinct bad payloads expose accidental copies.
     */
    reset();
    packet(1, 0x11, false);
    packet(2, 0xEE, true);
    packet(2, 0xDD, true);
    packet(2, 0x22, false);
    packet(3, 0x33, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 3u * XMODEM_BLOCK_SIZE, 3u * XMODEM_BLOCK_SIZE);
    filled(0, XMODEM_BLOCK_SIZE, 0x11);
    filled(XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, 0x22);
    filled(2u * XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, 0x33);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_NAK, XMODEM_NAK,
             XMODEM_ACK, XMODEM_ACK, XMODEM_ACK);
}

static void test_recovery(void)
{
    reset();
    event(-1);
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    CONTROLS(XMODEM_CRC, XMODEM_CRC, XMODEM_ACK, XMODEM_ACK);
    reset();
    packet(1, 3, false);
    wire.input[2] ^= 1;
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 9);
    CONTROLS(XMODEM_CRC, XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);
    reset();
    packet(0, 3, false);
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 9);
    CONTROLS(XMODEM_CRC, XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);
    /* A successful block resets the retry budget. */
    reset();
    for (unsigned i = 1; i < XMODEM_MAX_RETRIES; ++i)
        event(-1);
    packet(1, 8, false);
    for (unsigned i = 1; i < XMODEM_MAX_RETRIES; ++i)
        event(-1);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
}

static void test_partial_packet_timeout(void)
{
    /* Every missing body byte, from block number through the final CRC byte,
     * must abort immediately, both before and after an accepted block.
     */
    for (unsigned accepted = 0; accepted <= 1; ++accepted)
    {
        for (size_t cut = 1; cut < XMODEM_PACKET_SIZE; ++cut)
        {
            reset();
            if (accepted)
                packet(1, 0x11, false);
            size_t prefix = wire.length;
            packet((uint8_t)(accepted + 1u), 0x22, false);
            wire.length = prefix + cut;
            event(-1);
            run(XMODEM_ERR_TIMEOUT, 2u * XMODEM_BLOCK_SIZE, 0);
            assert(wire.reads == prefix + cut + 1u);
            filled(0, XMODEM_BLOCK_SIZE, accepted ? 0x11 : 0xA5);
            filled(XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, 0xA5);
            if (accepted)
                CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_CAN, XMODEM_CAN);
            else
                CONTROLS(XMODEM_CRC, XMODEM_CAN, XMODEM_CAN);
        }
    }

    /* Late bytes resembling control characters must remain unread after abort. */
    reset();
    event(XMODEM_SOH);
    event(1);
    event(-1);
    event(XMODEM_EOT);
    event(XMODEM_CAN);
    packet(1, 9, false);
    assert(xmodem_receive(&io, storage + 1, XMODEM_BLOCK_SIZE,
                          &received) == XMODEM_ERR_TIMEOUT);
    assert(received == 0 && wire.position == 3 && wire.reads == 3);
    for (size_t i = 0; i < sizeof storage; ++i)
        assert(storage[i] == 0xA5);
    CONTROLS(XMODEM_CRC, XMODEM_CAN, XMODEM_CAN);
}

static void test_between_packet_timeout(void)
{
    /* Unlike a partial packet, silence between packets allows retransmission. */
    reset();
    packet(1, 0x11, false);
    event(-1);
    packet(2, 0x22, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 256, 256);
    filled(0, 128, 0x11);
    filled(128, 128, 0x22);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);

    reset();
    packet(1, 0x11, false);
    for (unsigned i = 0; i < XMODEM_MAX_RETRIES; ++i)
        event(-1);
    run(XMODEM_ERR_TIMEOUT, 128, 0);
    filled(0, 128, 0x11);
    assert(wire.writes == XMODEM_MAX_RETRIES + 3u);
    assert(wire.output[0] == XMODEM_CRC && wire.output[1] == XMODEM_ACK);
    for (size_t i = 2; i < XMODEM_MAX_RETRIES + 1u; ++i)
        assert(wire.output[i] == XMODEM_NAK);
    assert(wire.output[wire.writes - 2] == XMODEM_CAN);
    assert(wire.output[wire.writes - 1] == XMODEM_CAN);
}

static void test_retry_exhaustion(void)
{
    /* Pin this regression to the specified limit, independently of the driver. */
    enum { CORRUPTED_COPIES = 16 };
    reset();
    for (unsigned i = 0; i < CORRUPTED_COPIES; ++i)
        packet(1, 3, true);

    run(XMODEM_ERR_CRC, XMODEM_BLOCK_SIZE, 0);
    filled(0, XMODEM_BLOCK_SIZE, 0xA5);
    /* Return immediately after the final packet, without waiting for timeout. */
    assert(wire.reads == CORRUPTED_COPIES * XMODEM_PACKET_SIZE);

    /* C, then 15 NAKs, then CAN CAN; no ACK or final NAK. */
    assert(wire.writes == CORRUPTED_COPIES + 2u);
    assert(wire.output[0] == XMODEM_CRC);
    for (size_t i = 1; i < CORRUPTED_COPIES; ++i)
        assert(wire.output[i] == XMODEM_NAK);
    assert(wire.output[CORRUPTED_COPIES] == XMODEM_CAN);
    assert(wire.output[CORRUPTED_COPIES + 1u] == XMODEM_CAN);
}

static void test_failures(void)
{
    reset();
    run(XMODEM_ERR_TIMEOUT, 128, 0);
    retry_controls(XMODEM_CRC);
    filled(0, 128, 0xA5);
    reset();
    for (unsigned i = 0; i < XMODEM_MAX_RETRIES; ++i)
        packet(2, 3, false);
    run(XMODEM_ERR_SEQUENCE, 128, 0);
    retry_controls(XMODEM_NAK);
    reset();
    for (unsigned i = 0; i < XMODEM_MAX_RETRIES; ++i)
        event(XMODEM_EOT);
    run(XMODEM_ERR_SEQUENCE, 128, 0);
    retry_controls(XMODEM_CRC);
    reset();
    for (unsigned i = 0; i < XMODEM_MAX_RETRIES; ++i)
        event(0x02);
    run(XMODEM_ERR_SEQUENCE, 128, 0);
    retry_controls(XMODEM_CRC);
}

static void test_sender_cancellation(void)
{
    /* Two consecutive CAN bytes cancel before any payload is accepted. */
    reset();
    event(XMODEM_CAN);
    event(XMODEM_CAN);
    run(XMODEM_ERR_CANCELLED, 128, 0);
    filled(0, 128, 0xA5);
    CONTROLS(XMODEM_CRC);
    reset();
    packet(1, 7, false);
    event(XMODEM_CAN);
    event(XMODEM_CAN);
    /* Current API reports size only on success; already-written bytes remain. */
    run(XMODEM_ERR_CANCELLED, 128, 0);
    filled(0, 128, 7);
    CONTROLS(XMODEM_CRC, XMODEM_ACK);
    reset();
    event(XMODEM_CAN);
    event(-1);
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    CONTROLS(XMODEM_CRC, XMODEM_CRC, XMODEM_ACK, XMODEM_ACK);

    /* A lone CAN during a transfer requests retry, not cancellation. */
    reset();
    packet(1, 0x11, false);
    event(XMODEM_CAN);
    event(-1);
    packet(2, 0x22, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 256, 256);
    filled(0, 128, 0x11);
    filled(128, 128, 0x22);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_NAK, XMODEM_ACK, XMODEM_ACK);

    /* CAN followed by unrelated noise is not a cancellation pair. */
    reset();
    event(XMODEM_CAN);
    event(0x55);
    packet(1, 9, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, 9);
    CONTROLS(XMODEM_CRC, XMODEM_CRC, XMODEM_ACK, XMODEM_ACK);

    /* CAN bytes inside a packet are ordinary payload, even consecutively. */
    reset();
    packet(1, XMODEM_CAN, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 128, 128);
    filled(0, 128, XMODEM_CAN);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK);

}

static void test_buffer_overflow(void)
{
    /* No partial block may be copied, including when capacity is zero. */
    for (size_t capacity = 0; capacity < XMODEM_BLOCK_SIZE; ++capacity)
    {
        reset();
        packet(1, 7, false);
        run(XMODEM_ERR_OVERFLOW, capacity, 0);
        filled(0, capacity, 0xA5);
        CONTROLS(XMODEM_CRC, XMODEM_CAN, XMODEM_CAN);
    }
    /* Preserve the first block and unused tail when the second will not fit.
     * run() also verifies the guard bytes beyond the supplied capacity.
     */
    for (size_t capacity = XMODEM_BLOCK_SIZE;
         capacity < 2u * XMODEM_BLOCK_SIZE; ++capacity)
    {
        reset();
        packet(1, 7, false);
        packet(2, 8, false);
        run(XMODEM_ERR_OVERFLOW, capacity, 0);
        filled(0, XMODEM_BLOCK_SIZE, 7);
        filled(XMODEM_BLOCK_SIZE, capacity - XMODEM_BLOCK_SIZE, 0xA5);
        CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_CAN, XMODEM_CAN);
    }

    /* Boundary control: an exact fit must succeed rather than cancel. */
    reset();
    packet(1, 7, false);
    packet(2, 8, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, 2u * XMODEM_BLOCK_SIZE, 2u * XMODEM_BLOCK_SIZE);
    filled(0, XMODEM_BLOCK_SIZE, 7);
    filled(XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, 8);
    CONTROLS(XMODEM_CRC, XMODEM_ACK, XMODEM_ACK, XMODEM_ACK);
}

static void test_sequence_rollover(void)
{
    /* Receive 257 distinct positions: sequence numbers 1..255, 0, 1.
     * Duplicate 255 while expecting 0, then duplicate 0 while expecting 1.
     */
    reset();
    for (unsigned i = 1; i <= 257; ++i)
    {
        uint8_t block = (uint8_t)i;
        /* Distinguish the final block 1 from the initial block 1. */
        uint8_t value = (i == 257) ? 0xA7u : block;
        packet(block, value, false);
        if (i == 255 || i == 256)
            packet(block, value, false);
    }
    event(XMODEM_EOT);
    run(XMODEM_OK, BUFFER_SIZE, BUFFER_SIZE);
    for (unsigned i = 1; i <= 257; ++i)
    {
        uint8_t value = (i == 257) ? 0xA7u : (uint8_t)i;
        filled((i - 1u) * XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, value);
    }
    /* Initial C, 257 data ACKs, two duplicate ACKs, and EOT ACK. */
    assert(wire.writes == 261 && wire.output[0] == XMODEM_CRC);
    for (size_t i = 1; i < wire.writes; ++i)
        assert(wire.output[i] == XMODEM_ACK);

    /* Block 1 must not be accepted before block 0 after the rollover. */
    reset();
    for (unsigned i = 1; i <= 255; ++i)
        packet((uint8_t)i, (uint8_t)i, false);
    packet(1, 0xEE, false); /* Out of sequence: request retransmission. */
    packet(0, 0, false);
    packet(1, 0xA7, false);
    event(XMODEM_EOT);
    run(XMODEM_OK, BUFFER_SIZE, BUFFER_SIZE);
    for (unsigned i = 1; i <= 256; ++i)
        filled((i - 1u) * XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, (uint8_t)i);
    filled(256u * XMODEM_BLOCK_SIZE, XMODEM_BLOCK_SIZE, 0xA7);
    assert(wire.writes == 260 && wire.output[0] == XMODEM_CRC);
    for (size_t i = 1; i < wire.writes; ++i)
        assert(wire.output[i] == (i == 256 ? XMODEM_NAK : XMODEM_ACK));
}

int main(void)
{
    test_arguments();
    test_success();
    test_duplicate_block();
    test_crc_retransmission();
    test_recovery();
    test_partial_packet_timeout();
    test_between_packet_timeout();
    test_retry_exhaustion();
    test_failures();
    test_sender_cancellation();
    test_buffer_overflow();
    test_sequence_rollover();
    puts("XMODEM receive tests passed");
    return 0;
}
