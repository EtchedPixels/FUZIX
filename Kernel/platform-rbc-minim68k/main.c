#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>
#include <devtty.h>

uint16_t swap_dev = 0xFFFF;

void do_beep(void)
{
}

/*
 *	MMU initialize
 */

void map_init(void)
{
}

uaddr_t ramtop;
uint8_t need_resched;

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
	kmemaddblk((void *)e, (ramsize << 10) - e);

	kprintf("Motorola 680%s%d processor detected.\n",
		sysinfo.cpu[1]>9?"":"0",sysinfo.cpu[1]);
	enable_icache();
	display_uarts();
}

/* Udata and kernel stacks */
u_block udata_block[PTABSIZE];

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x40);
}

uint8_t plt_udata_set(ptptr p)
{
	p->p_udata = &udata_block[p - ptab].u_d;
	return 0;
}

/* Fuzix platform interrupt: effectively the timer */

void plt_interrupt(void)
{
	static uint16_t counter;
	static uint8_t display;

	if (++counter >= TICKSPERSEC/2) {
		counter = 0;
		if (display == 0) {
			display = 1;
		}
		out(-1, display);
		display <<= 1;
	}

	timer_interrupt();

}

/* Early initialization C hook */
void c_init_hardware(void)
{
	/* Register the MF/PIC UART to use as our console */
	register_uart((void *)0xFFFF0048, &ns16x50_uart);
}

unsigned long long plt_arith(uint32_t days, uint32_t sec)
{
	return days*86400LL + sec;
}

void plt_daytime_setup(time_t *tm)
{
/*  25202 = Julian day 1/1/1970 minus Julian day 1/1/1901     */
/*  This fudge factor converts from BIOS time to Fuzix epoch  */
/*  fudge factor of 3 on time accounts for start-up delay in Fuzix */

__asm__ (
	" move.w #20,%d0;"
	" movq.l #1,%d1;"
	" trap   #8;"
	" sub.l  #25202,%d0;"
	" addq.l #3,%d1;"
	" move.l %d1,-(%sp);"
	" move.l %d0,-(%sp);"
	" bsr	plt_arith;"
	" lea	8(%sp),%sp;"
	
	" move.l 4(%sp),%a0;"
	" move.l %d1,(%a0);"
	" move.l %d0,4(%a0);"
	);
}
