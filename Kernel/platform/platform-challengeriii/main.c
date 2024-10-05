#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <devtty.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;
uint8_t piamask;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 *
 *	The SBCv2 has no interrupts so we must call sync_clock(), and as the
 *	PropIO tty is not interrupt driven we also poll the ttys. This gives
 *	us a nice interactive feel when the machine is idle, even if a polled
 *	tty can otherwise suck.
 */
void plt_idle(void)
{
	/* Disable interrupts so we don't accidentally process a polled tty
	   and interrupt call at once and make a mess */
	irqflags_t irq = di();
	tty_poll();
	/* Restore prior state. */
	irqrestore(irq);
}

/* Disk PIA has the timer tick */
static volatile uint8_t *piab = (volatile uint8_t *) 0xC010;
static uint8_t piac = 0xB1;

/* CA2 is a 400ms tick. Not ideal */
void plt_interrupt(void)
{
	tty_poll();
	/* 400ms tick - can use 40ms if change link and tweak kernel */
	if (piab[3] & 0x80) {
		/* We see one edge and then flip the pia to look for the other
		   thus getting 200ms tick which is less nasty */
		piac ^= 2;
		piab[3] = piac;
		piab[2];	/* Reset interrupt */
		timer_interrupt();
		timer_interrupt();
	}
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers. This blows away the _DISCARD segment.
 */
void plt_discard(void)
{
	uint16_t discard_size = 0xC000 - (uint16_t) bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset(bp, 0, discard_size * sizeof(struct blkbuf));

	for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}
