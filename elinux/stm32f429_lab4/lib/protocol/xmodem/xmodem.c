#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "lib/crc/crc16.h"
#include "lib/protocol/xmodem/xmodem.h"


/*
 * Send a control character through the caller's transport.
 */
static void xmodem_send(const xmodem_io_t *io, uint8_t value)
{
    io->write_byte(io->context, value);
}


/*
 * Cancel a transfer by sending CAN twice.
 */
static void xmodem_cancel(const xmodem_io_t *io)
{
    xmodem_send(io, XMODEM_CAN);
    xmodem_send(io, XMODEM_CAN);
}


/*
 * Read an XMODEM packet body.
 *
 * SOH has already been consumed by the caller.
 *
 * Packet format:
 *
 *   SOH         1 byte (already received)
 *   Block       1 byte
 *   ~Block      1 byte
 *   Data      128 bytes
 *   CRC high    1 byte
 *   CRC low     1 byte
 *
 * Returns false if any byte times out.
 */
static bool xmodem_read_packet(
    const xmodem_io_t *io,
    xmodem_packet_t *packet)
{
    /* SOH */
    packet->soh = XMODEM_SOH;

    /* Block number. */
    if (!io->read_byte(io->context, &packet->block, XMODEM_BYTE_TIMEOUT_MS))
        return false;

    /* Complement of block number. */
    if (!io->read_byte(io->context, &packet->block_inv, XMODEM_BYTE_TIMEOUT_MS))
        return false;

    /* Receive the 128-byte payload. */
    for (size_t i = 0; i < XMODEM_BLOCK_SIZE; ++i)
    {
        if (!io->read_byte(io->context, &packet->data[i], XMODEM_BYTE_TIMEOUT_MS))
            return false;
    }

    /* CRC16: high byte first. */
    if (!io->read_byte(io->context, &packet->crc_hi, XMODEM_BYTE_TIMEOUT_MS))
        return false;

    if (!io->read_byte(io->context, &packet->crc_lo, XMODEM_BYTE_TIMEOUT_MS))
        return false;

    return true;
}


/*
 * Validate the integrity of an XMODEM packet.
 *
 * Sequence ordering is deliberately NOT checked here.
 * That belongs to the receiver state machine.
 */
xmodem_packet_status_t xmodem_packet_validate(
    const xmodem_packet_t *packet)
{
    if (packet == NULL || packet->soh != XMODEM_SOH)
        return XMODEM_PACKET_ERR_FORMAT;

    /* Verify block number complement. */
    if ((uint8_t)(packet->block ^ packet->block_inv) != 0xFFu)
        return XMODEM_PACKET_ERR_BLOCK;

    /* CRC16 covers the 128-byte payload only. */
    uint16_t calculated_crc =
        crc16_xmodem(packet->data, XMODEM_BLOCK_SIZE);

    uint16_t received_crc =
        ((uint16_t)packet->crc_hi << 8) |
        packet->crc_lo;

    if (calculated_crc != received_crc)
        return XMODEM_PACKET_ERR_CRC;

    return XMODEM_PACKET_OK;
}


/*
 * Receive a file using classic 128-byte XMODEM-CRC.
 *
 * The destination buffer belongs to the caller.
 *
 * received_size includes any padding in the final
 * 128-byte XMODEM block.
 */
xmodem_status_t xmodem_receive(
    const xmodem_io_t *io,
    uint8_t *buffer,
    size_t capacity,
    size_t *received_size)
{
    /* Validate arguments */
    if (io == NULL || io->read_byte == NULL || io->write_byte == NULL)
        return XMODEM_ERR_IO;

    if (buffer == NULL || received_size == NULL)
        return XMODEM_ERR_IO;

    *received_size = 0u;
    
    xmodem_packet_t packet;

    uint8_t expected_block = 1u;
    uint32_t retries = 0u;

    size_t offset = 0u;

    bool transfer_started = false;

    xmodem_status_t last_error = XMODEM_ERR_TIMEOUT;

    /*
     * Ask the sender to use CRC16 mode.
     *
     * ASCII 'C' = 0x43.
     */
    xmodem_send(io, XMODEM_CRC);

    for (;;)
    {
        uint8_t control = 0u;

        /*
         * Read the next packet's control byte.
         */
        if (!io->read_byte(
                io->context,
                &control,
                XMODEM_START_TIMEOUT_MS))
        {
            last_error = XMODEM_ERR_TIMEOUT;
        }

        /*
         * EOT: end of transmission.
         */
        else if (control == XMODEM_EOT)
        {
            /*
             * Do not accept an empty transfer.
             */
            if (!transfer_started || offset == 0u)
            {
                last_error = XMODEM_ERR_SEQUENCE;
            }
            else
            {
                xmodem_send(io, XMODEM_ACK);

                *received_size = offset;

                return XMODEM_OK;
            }
        }

        /*
         * CAN: sender requests cancellation.
         */
        else if (control == XMODEM_CAN)
        {
            uint8_t second = 0u;

            /*
             * Require two consecutive CAN bytes
             * to avoid cancelling on a single
             * corrupted byte.
             */
            if (io->read_byte(
                    io->context,
                    &second,
                    XMODEM_BYTE_TIMEOUT_MS) &&
                second == XMODEM_CAN)
            {
                return XMODEM_ERR_CANCELLED;
            }

            last_error = XMODEM_ERR_SEQUENCE;
        }

        /*
         * SOH: start of a 128-byte data packet.
         */
        else if (control == XMODEM_SOH)
        {
            transfer_started = true;

            /*
             * Read remaining packet fields.
             */
            if (!xmodem_read_packet(io, &packet))
            {
                /* Framing is lost: late payload bytes must not be interpreted
                 * as control bytes for a new packet. Abort this transfer.
                 */
                xmodem_cancel(io);
                return XMODEM_ERR_TIMEOUT;
            }
            else
            {
                /*
                 * Verify complement and CRC16.
                 */
                xmodem_packet_status_t status =
                    xmodem_packet_validate(&packet);

                if (status == XMODEM_PACKET_OK)
                {
                    /*
                     * Case 1: Expected new block.
                     */
                    if (packet.block == expected_block)
                    {
                        /*
                         * Validate remaining capacity
                         * before copying.
                         */
                        if (offset > capacity ||
                            XMODEM_BLOCK_SIZE > capacity - offset)
                        {
                            xmodem_cancel(io);
                            return XMODEM_ERR_OVERFLOW;
                        }

                        memcpy(
                            buffer + offset,
                            packet.data,
                            XMODEM_BLOCK_SIZE);

                        offset += XMODEM_BLOCK_SIZE;

                        /*
                         * 8-bit block number wraps
                         * naturally from 255 to 0.
                         */
                        expected_block =
                            (uint8_t)(expected_block + 1u);

                        retries = 0u;

                        xmodem_send(io, XMODEM_ACK);

                        continue;
                    }

                    /*
                     * Case 2: Duplicate previous block.
                     *
                     * This can happen when the sender
                     * did not receive our previous ACK.
                     */
                    uint8_t previous_block =
                        (uint8_t)(expected_block - 1u);

                    if (offset != 0u &&
                        packet.block == previous_block)
                    {
                        /*
                         * ACK again without storing
                         * duplicate data.
                         */
                        retries = 0u;

                        xmodem_send(io, XMODEM_ACK);

                        continue;
                    }

                    /*
                     * Case 3: Unexpected block number.
                     */
                    last_error = XMODEM_ERR_SEQUENCE;
                }
                else if (status == XMODEM_PACKET_ERR_CRC)
                {
                    last_error = XMODEM_ERR_CRC;
                }
                else
                {
                    last_error = XMODEM_ERR_SEQUENCE;
                }
            }
        }

        /*
         * Unexpected control byte.
         *
         * This implementation does not support STX
         * (1024-byte XMODEM-1K packets).
         */
        else
        {
            last_error = XMODEM_ERR_SEQUENCE;
        }

        /*
         * Reaching here means a timeout,
         * malformed packet, or sequence error.
         */
        retries++;

        if (retries >= XMODEM_MAX_RETRIES)
        {
            xmodem_cancel(io);

            return last_error;
        }

        /*
         * Before receiving the first packet,
         * repeat 'C' to request CRC mode.
         *
         * After the transfer begins, request
         * retransmission using NAK.
         */
        if (!transfer_started)
        {
            xmodem_send(io, XMODEM_CRC);
        }
        else
        {
            xmodem_send(io, XMODEM_NAK);
        }
    }
}