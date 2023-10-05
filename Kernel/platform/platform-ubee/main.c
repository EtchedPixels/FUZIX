#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ubee.h>

uint8_t ubee_parallel;

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
	if (ubee_model == UBEE_256TC)
		__asm halt __endasm;
	else {
		/* Try to make the keyboard suck as little as possible */
		irqflags_t irq = di();
		lpen_kbd_poll();
		irqrestore(irq);
	}
}

void do_beep(void)
{
}

static uint8_t has_rtc;

__sfr __at 0x02 pia0b;
__sfr __at 0x04 cmos_reg;
__sfr __at 0x07 cmos_read;

uint8_t plt_rtc_secs(void)
{
	cmos_reg = 0x00;
	return cmos_read;
}

/*
 *	PIA0 bit 7 is variously connected to
 *	- RTC
 *	- Vertical blank
 *	- Net
 *	- Nothing (well actually a pull up resistor)
 *
 *	On machines with a vertical blank connection we get a 50Hz timer
 *	from the PIA transitions. On machines with an RTC then the RTC is
 *	normally on the pia bit. We only care about the RTC because the
 *	machines we support normally don't have a vblank rtc
 */
void plt_interrupt(void)
{
	static uint8_t icount;
	uint8_t r = pia0b;
	/* TODO: printer interrupt */
	if (ubee_model == UBEE_256TC && (r & 0x02))
		kbd_interrupt();
	if (r & 0x80) {
		cmos_reg = 0x0C;
		if (cmos_read & 0x40) {
			icount++;
			timer_interrupt();
			/* Turn 8Hz into 10Hz */
			if (icount == 4) {
				timer_interrupt();
				icount = 0;
			}
		}
		if (ubee_model != UBEE_256TC)
			lpen_kbd_poll();
	}
}

struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers.
 */
void plt_discard(void)
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

uint8_t disk_type[2];
