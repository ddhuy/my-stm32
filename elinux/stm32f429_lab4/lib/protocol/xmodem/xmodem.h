
#ifndef LIB_XMODEM_H
#define LIB_XMODEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* XMODEM protocol control characters. */
#define XMODEM_SOH  0x01u
#define XMODEM_EOT  0x04u
#define XMODEM_ACK  0x06u
#define XMODEM_NAK  0x15u
#define XMODEM_CAN  0x18u
#define XMODEM_CRC  0x43u  /* ASCII 'C' */

/* Classic XMODEM payload size. */
#define XMODEM_BLOCK_SIZE 128u
#define XMODEM_PACKET_SIZE (3u + XMODEM_BLOCK_SIZE + 2u)

/* XMODEM configuration constants */
#define XMODEM_BYTE_TIMEOUT_MS     1000u
#define XMODEM_START_TIMEOUT_MS    1000u
#define XMODEM_MAX_RETRIES         16u

typedef enum
{
    XMODEM_OK = 0,
    XMODEM_ERR_TIMEOUT,
    XMODEM_ERR_CRC,
    XMODEM_ERR_SEQUENCE,
    XMODEM_ERR_OVERFLOW,
    XMODEM_ERR_CANCELLED,
    XMODEM_ERR_IO

} xmodem_status_t;

typedef enum
{
    XMODEM_PACKET_OK = 0,
    XMODEM_PACKET_ERR_FORMAT,
    XMODEM_PACKET_ERR_BLOCK,
    XMODEM_PACKET_ERR_CRC

} xmodem_packet_status_t;

typedef struct
{
    uint8_t soh;
    uint8_t block;
    uint8_t block_inv;
    uint8_t data[XMODEM_BLOCK_SIZE];
    uint8_t crc_hi;
    uint8_t crc_lo;

} xmodem_packet_t;

/* Make sure that there is no padding in the struct */
_Static_assert(sizeof(xmodem_packet_t) == 133u, "Invalid XMODEM packet layout");

typedef struct
{
    /*
     * Receive one byte before timeout_ms expires.
     * Returns true if successful, false on timeout.
     */
    bool (*read_byte)(void *context,
                      uint8_t *byte,
                      uint32_t timeout_ms);

    /* Send one byte. */
    void (*write_byte)(void *context, uint8_t byte);

    void *context;

} xmodem_io_t;

/*
 * Receive XMODEM data into a caller-provided buffer.
 *
 * Timeouts waiting for a packet are retried up to XMODEM_MAX_RETRIES.
 * Once SOH is received, any packet-body timeout sends CAN CAN and returns
 * XMODEM_ERR_TIMEOUT immediately; the caller must discard/resynchronize any
 * late transport input before starting another transfer or command session.
 *
 * capacity: maximum writable bytes
 * received_size: number of bytes written, including
 *                any padding in the final XMODEM block
 */
xmodem_status_t xmodem_receive(
    const xmodem_io_t *io,
    uint8_t *buffer,
    size_t capacity,
    size_t *received_size);

/*
 * Validate a complete 133-byte XMODEM-CRC packet.
 *
 * expected_block is the next expected block number.
 * The caller owns the packet buffer.
 */
xmodem_packet_status_t xmodem_packet_validate(
    const xmodem_packet_t *packet);
    
#endif
