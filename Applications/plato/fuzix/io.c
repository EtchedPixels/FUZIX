/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.c - Input/output functions (serial/ethernet) (apple2 specific)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "../io.h"
#include "../screen.h"

extern int io_fd;
static char buf[64];
static char *bufp = buf;

void io_process_queue(void)
{
	int len;

	if (buf == bufp)
		return;
	len = write(io_fd, buf, bufp-buf);
	if (len == 0 || (len == -1 && errno == EAGAIN))
		return;
	if (len < 0) {
		perror("write");
		return;
	}
	/* Circular buffers are elegant, ldir is small and faster for 64 bytes */
	memmove(buf, buf + len, len);
	bufp -= len;
}

/**
 * io_send_byte(b) - Send specified byte out
 */
void io_send_byte(uint8_t b)
{
	/* We need to do proper buffering/queueing here ! */
	if (bufp != &buf[64])
		*bufp++ = b;
	else
		screen_beep();
}
