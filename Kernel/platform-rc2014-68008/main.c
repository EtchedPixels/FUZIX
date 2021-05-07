#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

uint16_t swap_dev = 0xFFFF;
uint8_t timer_source;
uint8_t nuart;
uint8_t quart_present;
uint8_t sc26c92_present;

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

uint8_t platform_param(char *p)
{
	return 0;
}

void platform_discard(void)
{
}

void memzero(void *p, usize_t len)
{
	memset(p, 0, len);
}

/* Some of this could be discard when we add that but we are not short of RAM
   in the same way on 68K */

static uint8_t probe_16x50(uint8_t p)
{
	uint8_t r;
	uint8_t lcr = in(p + 3);
	out(p + 3, lcr | 0x80);
	out(p + 1, 0xAA);
	if (in(p + 1) != 0xAA) {
		out(p + 3, lcr);
		return 0;
	}
	out (p + 3, lcr);
	if (in(p + 1) == 0xAA)
		return 0;

	out (p + 2, 0xE7);
	r = in(p + 2);
	if (r & 0x40) {
		/* Decode types with FIFO */
		if (r & 0x80) {
			if (r & 0x20)
				return 7;
			return 5;	/* 16550A */
		}
		/* Should never find this real ones were discontinued
		   very early due to a hardware bug */
		return 0;
	} else {
		/* Decode types without FIFO */
		out(p + 7, 0x2A);
		if (in (p + 7) == 0x2A)
			return 4;
		return 8;
	}
}

/* Look for a QUART at 0xBA */

#define QUARTREG(x)	((uint16_t)(((x) << 11) | 0xBA))

#define MRA	0x00
#define CRA	0x02
#define IMR1	0x05
#define CRB	0x0A
#define IVR1	0x0C
#define CRC	0x12
#define ACR2	0x14
#define IMR2	0x15
#define CTU2	0x16
#define CTL2	0x17
#define CRD	0x1A
#define IVR2	0x1C
#define STCT2	0x1E	/* Read... */

static uint8_t probe_quart(void)
{
	uint8_t c = in16(QUARTREG(IVR1));

	c++;
	/* Make sure we they don't appear to affect one another */
	out16(QUARTREG(IVR1), c);

	if(in16(QUARTREG(IVR1)) != c)
		return 0;

	if(in16(QUARTREG(MRA)) == c)
		return 0;

	/* Ok now check IVR2 also works */
	out16(QUARTREG(IVR2), c + 1);
	if(in16(QUARTREG(IVR1)) != c)
		return 0;
	if(in16(QUARTREG(IVR2)) != c + 1)
		return 0;
	/* OK initialize things so we don't make a nasty mess when we
	   get going. We don't want interrupts for anything but receive
	   at this point. We can add line change later etc */
	out16(QUARTREG(IMR1), 0x22);
	out16(QUARTREG(IMR2), 0x22);
	/* Ensure active mode */
	out16(QUARTREG(CRA), 0xD0);
	/* Clocking */
	out16(QUARTREG(CRC), 0xD0);
	/* Reset the channels */
	out16(QUARTREG(CRA), 0x20);
	out16(QUARTREG(CRA), 0x30);
	out16(QUARTREG(CRA), 0x40);
	out16(QUARTREG(CRB), 0x20);
	out16(QUARTREG(CRB), 0x30);
	out16(QUARTREG(CRB), 0x40);
	out16(QUARTREG(CRC), 0x20);
	out16(QUARTREG(CRC), 0x30);
	out16(QUARTREG(CRC), 0x40);
	out16(QUARTREG(CRD), 0x20);
	out16(QUARTREG(CRD), 0x30);
	out16(QUARTREG(CRD), 0x40);
	/* We need to set ACR1/ACR2 once we do rts/cts and modem lines right */
	return 1;
}

/* Our counter counts clock/16 so 460800 clocks/second */

static void quart_clock(void)
{
	/* Timer, clock / 16 */
	out16(QUARTREG(ACR2), 0x70);	/* Adjust for RTS/CTS too */
	/* 10 ticks per second */
	out16(QUARTREG(CTL2), 11520 & 0xFF);
	out16(QUARTREG(CTU2), 11520 >> 8);
	/* Timer interrupt also wanted */
	out16(QUARTREG(IMR2), 0x22 | 0x08);
	/* Timer on */
	in16(QUARTREG(STCT2));
	/* Tell the quart driver to do do timer ticks */
	timer_source = TIMER_QUART;
	kputs("quart clock enabled\n");
}

#define SC26C92_MRA	0
#define SC26C92_CRA	2
#define SC26C92_ACR	4
#define SC26C92_IMR	5
#define SC26C92_CTU	6
#define SC26C92_CTL	7
#define SC26C92_MRB	8
#define SC26C92_CRB	10
#define SC26C92_START	14
#define SC26C92_STOP	15

static uint8_t probe_sc26c92(uint8_t port)
{
	/* These dummy reads for timer control reply FF */
	if (in(port + SC26C92_START) != 0xFF ||
		in(port + SC26C92_STOP) != 0xFF)
		return 0;

	out(port + SC26C92_ACR, 0x30);	/* Count using the 7.37MHz clock */
	in(port + SC26C92_START);		/* Set it running */
	if (in(port + SC26C92_CTL) == in(port + SC26C92_CTL))	/* Reads should show different */
		return 0;		/* values */
	in(port + SC26C92_STOP);/* Stop the clock */
	if (in(port + SC26C92_CTL) != in(port + SC26C92_CTL))	/* and now same values */
		return 0;
	/* Ok looks like an SC26C92  */
	out(port + SC26C92_CRA, 0x10);	/* MR1 always */
	out(port + SC26C92_CRA, 0xB0);		/* MR0 on a 26C92, X bit control on a 88C681 */
	out(port + SC26C92_MRA, 0x00);		/* We write MR1/MR2 or MR0/1... */
	out(port + SC26C92_MRA, 0x01);
	out(port + SC26C92_CRA, 0x10);		/* MR1 */
	out(port + SC26C92_IMR, 0x22);
	if (SC26C92_MRA & 0x01) {
		/* SC26C92 */
		out(port + SC26C92_CRB, 0xB0);	/* Fix up MR0B, MR0A was done in the probe */
		out(port + SC26C92_MRB, 0x00);
		out(port + SC26C92_ACR, 0x80);	/* ACR on counter off */
		return 1;
	}
	/* 88C681 */
	out(port + SC26C92_ACR, 0x00);
	return 2;
}

static void sc26c92_timer(uint8_t port)
{
	if (sc26c92_present == 1)		/* SC26C92 */
		out(port + SC26C92_ACR, 0xB0);		/* Counter | ACR = 1 */
	else					/* 88C681 */
		out(port + SC26C92_ACR, 0x30);		/* Counter | ACR = 0 */
	out(port + SC26C92_CTL, 46080 & 0xFF);
	out(port + SC26C92_CTU, 46080 >> 8);
	in(port + SC26C92_START);
	out(port + SC26C92_IMR, 0x32);		/* Timer and both rx/tx */
	timer_source = TIMER_SC26C92;
}

void init_hardware_c(void)
{
	/* TODO: ACIA */
	if (probe_16x50(0xC0))
		register_uart(0xC0, &ns16x50_uart);
	if (probe_16x50(0xC8))
		register_uart(0xC8, &ns16x50_uart);
	sc26c92_present = probe_sc26c92(0xA0);
	switch(sc26c92_present) {
		case 1:
			register_uart(0x00A0, &sc26c92_uart);
			register_uart(0x00A8, &sc26c92_uart);
			if (timer_source == TIMER_NONE)
				sc26c92_timer(0xA0);
			kputs("SC26C92 detected at 0xA0.\n");
			break;
		case 2:
			register_uart(0x00A0, &xr88c681_uart);
			register_uart(0x00A8, &xr88c681_uart);
			if (timer_source == TIMER_NONE)
				sc26c92_timer(0xA0);
			kputs("XR88C681 detected at 0xA0.\n");
			break;
	}

	/* Probe hardware */
	quart_present = probe_quart();
	if (quart_present) {
		register_uart(0x00BA, &quart_uart);
		register_uart(0x40BA, &quart_uart);
		register_uart(0x80BA, &quart_uart);
		register_uart(0xC0BA, &quart_uart);
		/* If we don't have a TMS9918A then the QUART is the next
		   best clock choice */
		if (timer_source == TIMER_NONE)
			quart_clock();
	}
}

void pagemap_init(void)
{
	/* Linker provided end of kernel */
	/* TODO: create a discard area at the end of the image and start
	   there */
	extern uint8_t _end;
	uint32_t e = (uint32_t)&_end;

	/* Allocate the rest of memory to the userspace */
	kmemaddblk((void *)e, 0x100000 - e);

	kprintf("Motorola 680%s%d processor detected.\n",
		sysinfo.cpu[1]?"":"0",sysinfo.cpu[1]);
	enable_icache();
	display_uarts();
	/* Complete the timer set up */
	if (timer_source == TIMER_NONE)
		kputs("Warning: no timer available.\n");
}

/* Udata and kernel stacks */
/* We need an initial kernel stack and udata so the slot for init is
   set up at compile time */
u_block udata_block0;
static u_block *udata_block[PTABSIZE] = {&udata_block0,};

/* This will belong in the core 68K code once finalized */

void install_vdso(void)
{
	extern uint8_t vdso[];
	/* Should be uput etc */
	memcpy((void *)udata.u_codebase, &vdso, 0x40);
}

/* FIXME: this tends to leave fixed objects messing up the memory pool.
   Allocate them at boot instead */
uint8_t platform_udata_set(ptptr p)
{
	u_block **up = &udata_block[p - ptab];
	if (*up == NULL) {
		*up = kmalloc(sizeof(struct u_block), 0);
		if (*up == NULL)
			return ENOMEM;
	}
	p->p_udata = &(*up)->u_d;
	return 0;
}

void do_timer_interrupt(void)
{
	timer_interrupt();
}

void platform_interrupt(void)
{
	tty_pollirq();
}
