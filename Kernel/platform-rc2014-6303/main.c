#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devide.h>
#include <devtty.h>

uint8_t kernel_flag = 1;
uint8_t need_resched;
uint16_t swap_dev = 0xFFFF;

/* Onboard I/O */
static volatile uint8_t *cpuio = (volatile uint8_t *)0;

void platform_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
}

/*
 * Map handling: allocate 3 banks per process
 */

void pagemap_init(void)
{
    int i;
    /* 32-35 are the kernel, 36-38 / 39-41 / etc are user */
    /* Add 36-38 last as we hardcode 36 into our init creation in tricks.s */
    for (i = 8; i >= 0; i--)
        pagemap_add(36 + i * 3);
}

void map_init(void)
{
}

uint8_t platform_param(char *p)
{
    return 0;
}


/*
 *	We don't have a counter but a free running timer at system E clock
 *	rate (1.8432MHz) and compare register. We can reset the timer but
 *	that horks the serial port so we have to play comparison games or
 *	just go with the timer. Now as it happens the timer wraps 28 times
 *	a second to within 1%. Good enough for scheduling but will need minor
 *	compensation logic for the clock later TODO
 */

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#endif
	/* We will just use the TOF event */
	cpuio[0x08] = 0x04;
}

/* FIXME: route serial interrupts directly in asm to a buffer and just
   queue process here */

static uint8_t tickmod;

void platform_interrupt(void)
{
	uint8_t dummy;

	tty_poll();

	/* Reading the register resets the flag if set */
	if (cpuio[0x08] & 0x20) {
		cpuio[9];	/* Read tmer to clear interrupt */
		tickmod++;
		if (tickmod == 7)
			tickmod = 0;
		/* Turn it into a 20Hz timer. For now do a simple fixup
		   for the 28->28 and ignore the extra 1% or so error */
		if (tickmod != 2 && tickmod != 6)
			timer_interrupt();
	}
}

/* For now this and the supporting logic don't handle swap */

extern uint8_t hd_map;
extern void hd_read_data(uint8_t *p);
extern void hd_write_data(uint8_t *p);

void devide_read_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_read_data(blk_op.addr);	
}

void devide_write_data(void)
{
	if (blk_op.is_user)
		hd_map = 1;
	else
		hd_map = 0;
	hd_write_data(blk_op.addr);	
}


/* C library equivalents */

void *memcpy(void *dest, const void *src, size_t len)
{
	uint8_t *dp = (uint8_t *)dest - 1;
	const uint8_t *sp = (uint8_t *)src - 1;
	while(len--)
		*++dp=*++sp;
	return dest;
}

void *memset(void *dest, int data, size_t len)
{
	char *p = dest;
	char v = (char)data;

	while(len--)
		*p++ = v;
	return dest;
}
