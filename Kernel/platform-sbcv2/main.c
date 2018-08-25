#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <rtc.h>
#include <ds1302.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void platform_idle(void)
{
	irqflags_t irq = di();
	sync_clock();
	tty_poll();
	irqrestore(irq);
}

void platform_interrupt(void)
{
	/* TODO */
	tty_poll();
}

struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers.
 */
void platform_discard(void)
{
	uint16_t discard_size = (uint16_t)udata - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}


/*
 *	Logic for tickless system. If you have an RTC you can ignore this.
 */

static uint8_t newticks = 0xFF;
static uint8_t oldticks;

static uint8_t re_enter;

void sync_clock_read(void)
{
	uint8_t s;
	oldticks = newticks;
	ds1302_read_clock(&s, 1);
	s = (s & 0x0F) + (((s & 0xF0) >> 4) * 10);
	newticks = s;
}

void sync_clock(void)
{
	irqflags_t irq = di();
	int16_t tmp;
	if (!re_enter++) {
		sync_clock_read();
		if (oldticks != 0xFF) {
			tmp = newticks - oldticks;
			if (tmp < 0)
				tmp += 60;
			tmp *= 10;
			while(tmp--) {
				timer_interrupt();
			}
			platform_interrupt();
		}
		re_enter--;
	} 
	irqrestore(irq);
}

void update_sync_clock(void)
{
}
