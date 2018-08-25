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

static uint8_t rtc_buf[8];

int platform_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (udata.u_count < len)
		len = udata.u_count;

	ds1302_read_clock(rtc_buf, 7);

	y = rtc_buf[6];
	if (y > 0x70)
		y = 0x1900 | y;
	else
		y = 0x2000 | y;
	*p++ = y >> 8;
	*p++ = y;
	rtc_buf[4]--;		/* 0 based */
	if ((rtc_buf[4] & 0x0F) > 9)	/* Overflow case */
		rtc_buf[4] -= 0x06;
	*p++ = rtc_buf[4];	/* Month */
	*p++ = rtc_buf[3];	/* Day of month */
	if ((rtc_buf[2] & 0x90) == 0x90) {	/* 12hr mode, PM */
		/* Add 12 BCD */
		rtc_buf[2] += 0x12;
		if ((rtc_buf[2] & 0x0F) > 9)	/* Overflow case */
			rtc_buf[2] += 0x06;
	}
	*p++ = rtc_buf[2];	/* Hour */
	*p++ = rtc_buf[1];	/* Minute */
	*p = rtc_buf[0];	/* Second */
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int platform_rtc_write(void)
{
	udata.u_error = -EOPNOTSUPP;
	return -1;
}

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
