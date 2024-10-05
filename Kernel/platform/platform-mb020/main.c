#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>
#include <blkdev.h>

uint16_t swap_dev = 0xFFFF;
uaddr_t ramtop;
uint8_t need_resched;


void do_beep(void)
{
}

/*
 *	MMU initialize
 */

void map_init(void)
{
}

/* Early initialization C hook */
void c_init_hardware(void)
{
	/* Register the UART to use as our console */
	register_uart((void *)0xFFFFF0C0, &ns16x50_uart);
}

uint8_t plt_param(char *p)
{
	return 0;
}

void plt_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

void pagemap_init(void)
{
	/* Linker provided end of kernel */
	/* TODO: create a discard area at the end of the image and start
	   there */
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;
	/* Allocate the rest of memory to the userspace */
	/* TODO: assumes 16MB */
	kmemaddblk((void *)e, 0x04000000 - e);

	kprintf("Motorola 680%s%d processor detected.\n",
		sysinfo.cpu[1]?"":"0",sysinfo.cpu[1]);
	enable_icache();
	display_uarts();
	ds1302_init();
}

/* Udata and kernel stacks */
/* We need an initial kernel stack and udata so the slot for init is
   set up at compile time */
u_block udata_block[PTABSIZE];

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x20);
}

uint8_t plt_udata_set(ptptr p)
{
	p->p_udata = &udata_block[p - ptab].u_d;
	return 0;
}

/*
 *	Timer interrupt
 */

void plt_interrupt(void)
{
	timer_interrupt();
	wakeup(&plt_interrupt);
}

/*
 *	RTC glue
 */

#define PIN_CE		0x10
#define PIN_DATA_HIZ	0x20
#define PIN_CLK		0x40
#define PIN_DATA_OUT	0x80
#define PIN_DATA_IN	0x01

static uint_fast8_t rtc_shadow;
#define rtc	(*((volatile uint8_t *)0xFFFFF00C))

uint_fast8_t ds1302_get_data(void)
{
	return rtc & PIN_DATA_IN;
}

void ds1302_set_driven(uint_fast8_t on)
{
	rtc_shadow &= ~PIN_DATA_HIZ;
	if (!on)
		rtc_shadow |= PIN_DATA_HIZ;
	rtc = rtc_shadow;
}

void ds1302_set_data(uint_fast8_t on)
{
	rtc_shadow &= ~PIN_DATA_OUT;
	if (on)
		rtc_shadow |= PIN_DATA_OUT;
	rtc = rtc_shadow;
}	

void ds1302_set_ce(uint_fast8_t on)
{
	rtc_shadow &= ~PIN_CE;
	if (on)
		rtc_shadow |= PIN_CE;
	rtc = rtc_shadow;
}	

void ds1302_set_clk(uint_fast8_t on)
{
	rtc_shadow &= ~PIN_CLK;
	if (on)
		rtc_shadow |= PIN_CLK;
	rtc = rtc_shadow;
}	
