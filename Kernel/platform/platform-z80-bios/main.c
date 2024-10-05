#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <rtc.h>
#include <bios.h>

uint16_t ramtop;		/* Updated at boot */
uint16_t udata_stash;
uint8_t swap_size;
uint16_t swap_dev = 0xFFFF;

uint8_t plt_tick_present;
uint8_t rtc_delay = 1;

struct fuzixbios_info *biosinfo;
uint8_t *alloc_base;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 */
void plt_idle(void)
{
	fuzixbios_idle();
}

/*
 *	Callbacks - these are called from the asm wrappers
 */
/*
 *	Called when the timer ticks. We should extend this to scale the
 *	timer nicely. Flags tells us useful things like if we are a vblank
 *	or a 1/10th tick etc
 */

void plt_interrupt(void)
{
	timer_interrupt();
}

uint16_t do_callback_timer(uint16_t event) __z88dk_fastcall
{
	used(event);
	return 0;
}

uint16_t do_callback_tty(uint16_t val) __z88dk_fastcall
{
	uint8_t tty = val;
	uint8_t op = tty >> 4;
	tty &= 0x0F;
	switch(op) {
	case 0:
		tty_inproc(tty, val >> 8);
		break;
	case 1:
		tty_carrier_raise(tty);
		break;
	case 2:
		tty_carrier_drop(tty);
		break;
	}
	return 0;
}

/*
 *	Allocate memory from the initialization pool
 *	Work down so that the BIOS can do discards later on
 */

void *init_alloc(uint16_t n)
{
	uint8_t *p = alloc_base - n;
	if (p < (uint8_t *)biosinfo->bios_top)
		return NULL;
	alloc_base = p;
	return p;
}

void *buffer_alloc(bufptr p)
{
	memset(p, 0, sizeof(*p));
	p->bf_dev = NO_DEVICE;
	p->bf_busy = BF_FREE;
	p->__bf_data = init_alloc(BLKSIZE);
	return p->__bf_data;
}

void buffer_init(void)
{
	bufptr p = bufpool_end;
	while(p < &bufpool[MAXBUFS]) {
		if (buffer_alloc(p) == NULL)
			break;
		p++;
	}
	bufpool_end = p;
}

struct blkbuf bufpool[MAXBUFS];
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers. This blows away the _DISCARD segment.
 */
void plt_discard(void)
{
	fuzixbios_init_done();
	buffer_init();
	/* TODO - allocate any remaining buffers we can from between
	   s__DISCARD and s__DISCARD + l__DISCARD */
}

/*
 *	Always called with interrupts off.
 */
uint8_t plt_rtc_secs(void)
{
	return fuzixbios_rtc_secs();
}
