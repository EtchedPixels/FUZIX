/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.h - Input/output functions (serial/ethernet)
 */

#ifndef IO_H
#define IO_H

/**
 * io_init() - Set-up the I/O
 */
void io_init(void);

/**
 * io_send_byte(b) - Send specified byte out
 */
void io_send_byte(uint8_t b);

/**
 * io_main() - The IO main loop
 */
void io_main(void);

/**
 * io_done() - Called to close I/O
 */
void io_done(void);

/**
 * io_process_queue() - Process the outbound queue
 */

void io_process_queue(void);

#endif /* IO_H */
