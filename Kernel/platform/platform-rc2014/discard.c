#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <ds12885.h>
#include <tinyide.h>
#include <tinysd.h>
#include <tinyscsi.h>
#include <ch375.h>
#include <rcbus.h>
#include <vt.h>
#include <netdev.h>
#include <zxkey.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include <graphics.h>
#include <devlpr.h>
#include <pcf8584.h>
#include "z180_uart.h"
#include "multivt.h"
#include "ncr5380.h"

/* Everything in here is discarded after init starts */

static void nap(void)
{
}

extern uint8_t fontdata_6x8[];

static const char *vdpname = "TMS9918A";	/* Could be 28 or 29 */

static uint8_t probe_tms9918a(void)
{
	uint16_t ct = 0;
	uint8_t v;

	/* Try turning it on and looking for a vblank */
	tms9918a_reset();

	/* Should see the top bit go high */
	do {
		v = tms9918a_ctrl & 0x80;
		/* On some systems the floating bus can confuse the high/low test so do extra
		   checking - also this speeds stuff up */
		/* We should never see C or 5S set as we have no sprites running */
		if (v & 0x60)
			return 0;
	} while(--ct && !(v & 0x80));

	if (ct == 0)
		return 0;

	nap();

	/* Reading the F bit should have cleared it */
	v = tms9918a_ctrl;
	if (v & 0x80)
		return 0;

	ct = 0;
	/* Now try and version detect : the TMS9918A IRQ must be off here */
	while(--ct && (tms9918a_ctrl & 0x80) == 0);
	if (ct == 0)
		return 0;

	v = tms_do_setup();
	switch(v) {
	case 1:
		v = HW_VDP_9918A;
		break;
	case 2:
		vdpname = "VDP938";
		v = HW_VDP_9938;
		break;
	case 3:
		vdpname = "VDP9958";
		v = HW_VDP_9958;
		break;
	}
	return v;
}

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

__sfr __at 0xA0 sc26c92_mra;
__sfr __at 0xA2 sc26c92_cra;
__sfr __at 0xA4 sc26c92_acr;
__sfr __at 0xA6 sc26c92_ctu;
__sfr __at 0xA7 sc26c92_ctl;
__sfr __at 0xA5 sc26c92_imr;
__sfr __at 0xA8 sc26c92_mrb;
__sfr __at 0xAA sc26c92_crb;
__sfr __at 0xAE sc26c92_start;
__sfr __at 0xAF sc26c92_stop;

static uint8_t probe_sc26c92(void)
{
	volatile uint8_t dummy;

	/* These dummy reads for timer control reply FF */
	if (sc26c92_start != 0xFF || sc26c92_stop != 0xFF)
		return 0;

	sc26c92_acr = 0x30;		/* Count using the 7.37MHz clock */
	dummy = sc26c92_start;		/* Set it running */
	if (sc26c92_ctl == sc26c92_ctl)	/* Reads should show different */
		return 0;		/* values */
	dummy = sc26c92_stop;		/* Stop the clock */
	if (sc26c92_ctl != sc26c92_ctl)	/* and now same values */
		return 0;
	/* Ok looks like an SC26C92  */
	sc26c92_cra = 0x10;		/* MR1 always */
	sc26c92_cra = 0xB0;		/* MR0 on a 26C92, X bit control on a 88C681 */
	sc26c92_mra = 0x00;		/* We write MR1/MR2 or MR0/1... */
	sc26c92_mra = 0x01;
	sc26c92_cra = 0x10;		/* MR1 */
	sc26c92_imr = 0x22;
	if (sc26c92_mra & 0x01) {
		/* SC26C92 */
		sc26c92_crb = 0xB0;	/* Fix up MR0B, MR0A was done in the probe */
		sc26c92_mrb = 0x00;
		sc26c92_acr = 0x80;	/* ACR on counter off */
		return 1;
	}
	/* 88C681 */
	sc26c92_acr = 0x00;
	return 2;
}

static void sc26c92_timer(void)
{
	volatile uint8_t dummy;
	if (sc26c92_present == 1)		/* SC26C92 */
		sc26c92_acr = 0xB0;		/* Counter | ACR = 1 */
	else					/* 88C681 */
		sc26c92_acr = 0x30;		/* Counter | ACR = 0 */
	sc26c92_ctl = 46080 & 0xFF;
	sc26c92_ctu = 46080 >> 8;
	dummy = sc26c92_start;
	sc26c92_imr = 0x32;		/* Timer and both rx/tx */
	timer_source = TIMER_SC26C92;
}

void init_hardware_c(void)
{
	ramsize = 512;
	procmem = 512 - 112;
	ef9345_present = ef9345_probe();
	if (ef9345_present) {
		shadowcon = 1;
		/* Need a cleaner setup for the video sizes */
		ef9345_init();
		vt_twidth = 80;
		vt_tright = 79;
		vt_theight = 24;
		vt_tbottom = 23;
		curvid = vidcard[1] = VID_EF9345;
		vtinit();
		/* TODO: ef9345 as vblank ?? */
	}

#ifdef CONFIG_RC2014_PROPGFX
	/* No probe for this one... */
	macca_present = macca_init();
#endif
	if (macca_present) {
		if (shadowcon == 0) {
			shadowcon = 1;
			vt_twidth = 40;
			vt_tright = 39;
			vt_theight = 30;
			vt_tbottom = 29;
			curvid = vidcard[1] = VID_MACCA;
			vtinit();
		}
		shadowcon = 1;
	}

	/* The TMS9918A and KIO at 0x80 clash */
	if (kio_port != 0x80) {
		tms9918a_present = probe_tms9918a();
		if (tms9918a_present) {
			tms9918a_reload();
			curvid = VID_TMS9918A;
			/* Set up but don't yet turn the interrupt on */
			timer_source = TIMER_TMS9918A;
			if (shadowcon == 0) {
				vt_twidth = 40;
				vt_tright = 39;
				vt_theight = 24;
				vt_tbottom = 23;
				vtinit();
				vidcard[1] = curvid;
			}
			shadowcon = 1;
		}
	}
	/* Set the right console for kernel messages */
	if (z180_present) {
		z180_setup(!ctc_port);
		register_uart(Z180_IO_BASE, &z180_uart0);
		register_uart(Z180_IO_BASE + 1, &z180_uart1);
		rtc_port = 0x0C;
		rtc_shadow = 0x0C;
		timer_source = TIMER_Z180;
	}
	if (systype == 9) {	/* Easy Z80 is different */
		register_uart(0x80, &easy_uart);
		register_uart(0x82, &easy_uartb);
	}
	if (eipc_present) {
		register_uart(0x18, &eipc_uart);
		register_uart(0x1A, &eipc_uart);
	}
	if (kio_port == 0x80) {
		register_uart(0x88, &kio_uart);
		register_uart(0x8C, &kio_uart);
	}
	/* ROMWBW favours the Z180, 16x50 then SIO then ACIA */
	if (u16x50_present) {
		register_uart(0xA0, &ns16x50_uart);
		if (probe_16x50(0xA8))
			register_uart(0xA8, &ns16x50_uart);
	}
	if (sio_present) {
		register_uart(0x80, &sio_uart);
		register_uart(0x82, &sio_uartb);
	}
	if (acia_present)
		register_uart(0xA0, &acia_uart);
	/* TODO: sc26c92 as boot probe if added to ROMWBW */
}

__sfr __at 0xBC copro_ack;
__sfr __banked __at 0xFFBC copro_boot;	/* INT, NMI reset high */
__sfr __banked __at 0xBC copro_reset;	/* reset low */

static uint8_t probe_copro(void)
{
	uint8_t i = 0;
	uint8_t c;

	copro_reset = 0x00;		/* Force a reset */
	while(i < 255)
		i++;

	copro_boot = 0x00;

	i = 0;
	while(i < 255 && copro_ack != 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;

	copro_boot = 0xFF;
	i = 0;
	while(i < 255 && copro_ack == 0xAA) {
		i++;
	}
	if (i == 255)
		return 0;
	/* Now read the banner and print it */
	while(1) {
		c = copro_ack;
		/* Ack the byte */
		copro_boot = 0x00;
		if (c == 0)
			break;
		/* While we print the char the coprocessor will get the next
		   one ready - and it will beat us to the next step */
		kputchar(c);
		c = copro_ack;
		copro_boot = 0x80;
		if (c == 0)
			break;
		kputchar(c);
	}
	kputchar('\n');
	return 1;
}

void vdu_setup(void)
{
	/* Work out how many TMS9918A to nail on the end */
	uint8_t num_tms = NUM_DEV_TTY - nuart + 1;
	if (num_tms > 4)
		num_tms = 4;
	if (ef9345_present)
		num_tms--;
	if (macca_present)
		num_tms -= 2;

	if (shadowcon) {
		uint8_t n = 0;
		shadowcon = 0;
		if (tms9918a_present) {
			while (n++ < num_tms)
				insert_uart(0x98, &tms_uart);
		}
		if (macca_present) {
			insert_uart(MACCA_BASE, &macca_uart);
			insert_uart(MACCA_BASE, &macca_uart);
		}
		if (ef9345_present)
#ifdef CONFIG_RC2014_EXTREME
			insert_uart(0x44B8, &ef_uart);
#else
			insert_uart(0x44, &ef_uart);
#endif
		n = 1;
		/* Now set the vidcard table up */
		if (ef9345_present)
			vidcard[n++] = VID_EF9345;
		if (macca_present) {
			vidcard[n++] = VID_MACCA;
			vidcard[n++] = VID_MACCA;
		}
		if (tms9918a_present) {
			while(num_tms--)
				vidcard[n++] = VID_TMS9918A;
		}
		/* Set the video state up. It's a bit scrambled at this
		   point so we just cycle the consoles to clean up */
		for (n = 1; n < 5; n++)
			do_conswitch(n);
		do_conswitch(1);
	}
}

__sfr __at 0xED z512_ctrl;

static char *sysname(void)
{
	if (systype == 7)
		return "n RC2014";
	if (systype == 8)
		return "n RCBUS Z180";
	if (systype == 9) {
		if (eipc_present)
			return " TinyZ80";
		return "n EasyZ80";
	}
	return "n unknown device";
}
/*
 *	Do the main memory bank and device set up
 */
void pagemap_init(void)
{
	uint8_t i, m;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 0x20-0x3F (page size is 16 KiB)
	 *
	 * 0x20: Kernel low 16K
	 * 0x21/0x22: Kernel banked 32K bank 1
	 * 0x23: Kernel/init top 16K
	 * 0x24/25: Kernel banked 32K bank 2
	 * 0x26/27: Kernel banked 32K bank 3 (only 0x26 used for now)
	 */
	for (i = 0x28; i < 0x40; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(0x23);

	kprintf("RomWBW %d.%d on a%s@%dMHz.\n",
		romver >> 4, romver & 0x0F,
		sysname(), syscpu & 0xFF);
#ifdef CONFIG_RTC_DS1302
	/* Could be at 0xC0 or 0x0C */
	ds1302_init();
	inittod();
	if (!ds1302_present) {
		rtc_port = 0x0C;
		ds1302_init();
	}
#endif

	quart_present = probe_quart();
	/* Further ports we register at this point */
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

	if (sio1_present) {
		register_uart(0x84, &sio_uart);
		register_uart(0x86, &sio_uartb);
	}

	/* TODO : we assume normal RC2014 speeds, or 16MHz for the EIPC
	   based machine. Possibly we should ask ROMWBW and pick from
	   a table for other speeds */
	if (ctc_port) {
		if (timer_source == TIMER_NONE) {
			timer_source = TIMER_CTC;
			if (eipc_present) {
				ticksperclk = 25;
				out(ctc_port + 2, 0xB5);
				out(ctc_port + 2, 250);
			} else {
				ticksperclk = 20;
				out(ctc_port + 2, 0xB5);
				out(ctc_port + 2, 144);
			}
		}
		kprintf("Z80 CTC detected at 0x%2x.\n", ctc_port);
	}
	/* Prefer CTC to DS12885 timer */
#ifdef CONFIG_RTC_DS12885
	ds12885_init();
	inittod();
	if (ds12885_present) {
		kprintf("DS12885 detected at 0x%x.\n", rtc_port);
		if (timer_source == TIMER_NONE) {
			ds12885_set_interval();
			timer_source = TIMER_DS12885;
			kputs("DS12885 timer enabled\n");
		} else {
			ds12885_disable_interval();
		}
	}
#endif

	if (tms9918a_present)
		kprintf("%s detected at 0x98.\n", vdpname);
	if (ef9345_present)
#ifdef CONFIG_RC2014_EXTREME
		kputs("EF9345 detected at 0x44B8.\n");
#else
		kputs("EF9345 detected at 0x44.\n");
#endif

	if (!acia_present)
		sc26c92_present = probe_sc26c92();

	if (sc26c92_present == 1) {
		register_uart(0x00A0, &sc26c92_uart);
		register_uart(0x00A8, &sc26c92_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
		kputs("SC26C92 detected at 0xA0.\n");
	}
	if (sc26c92_present == 2) {
		register_uart(0x00A0, &xr88c681_uart);
		register_uart(0x00A8, &xr88c681_uart);
		if (timer_source == TIMER_NONE)
			sc26c92_timer();
		kputs("XR88C681 detected at 0xA0.\n");
	}

	/* Complete the timer set up */
	if (timer_source == TIMER_NONE)
		kputs("Warning: no timer available.\n");
	else
		plt_tick_present = 1;

	dma_present = !probe_z80dma();
	if (dma_present)
		kputs("Z80DMA detected at 0x04.\n");

#ifdef CONFIG_RTC_DS1302
	if (ds1302_present)
		kprintf("DS1302 detected at 0x%x.\n", rtc_port);
#endif

	/* Boot the coprocessor if present (just one for now) */
	copro_present = probe_copro();
	if (copro_present)
		kputs("Z80 Co-processor at 0xBC\n");

	if (ps2port_init())
		ps2_type = PS2_DIRECT;
	else
		ps2_type = PS2_BITBANG;	/* Try bitbanger */

	ps2kbd_present = ps2kbd_init();
	if (ps2kbd_present) {
		kprintf("PS/2 Keyboard at 0x%2x\n", ps2_type == PS2_DIRECT ? 0x60 : 0xBB);
		if (!zxkey_present && shadowcon) {	/* TOOD: || ef9345 - test shadowcon ? */
			/* Add the consoles */
			kputs("Switching to video output.\n");
			vdu_setup();
		}
	}
	ps2mouse_present = ps2mouse_init();
	if (ps2mouse_present) {
		kprintf("PS/2 Mouse at 0x%2x\n", ps2_type == PS2_DIRECT ? 0x60 : 0xBB);
		/* TODO: wire to input layer and interrupt */
	}
	/* The PropGfx and FPU clash, or at least the non extreme version.If
	   we do an extreme propgfx setup on 16bit I/O we can sort this TODO */
#ifndef CONFIG_RC2014_EXTREME
	if (!macca_present && fpu_detect())
#else
	if (fpu_detect())
		kputs("AMD9511 FPU at 0x42\n");
#endif
	/* At 0xC0 the KIO is registered later as it won't be the system
	   console. This will need to change if RomWBW changes */
	if (kio_port == 0xC0) {
		register_uart(0xC8, &kio_uart);
		register_uart(0xCC, &kio_uart);
	}
	/* Devices in the C0-FF range cannot be used with Z180 */
	if (!z180_present) {
		i = 0xC0;
		if (kio_port == 0xC0)
			i = 0xE0;
		while(i) {
			if (
#ifdef CONFIG_RTC_DS1302
				!ds1302_present ||
#endif
#ifdef CONFIG_RTC_DS12885
				!ds12885_present ||
#endif
				rtc_port != i) {
				if (m = probe_16x50(i)) {
					register_uart(i, &ns16x50_uart);
					/* Can't be a Z80-512K if there is a
					   UART at 0xE8 */
					if (i == 0xE8)
						z512_present = 0;
				}
			}
			i += 0x08;
		}
		/* Now check for Z80-512K if still possible */
		if (z512_present) {
			z512_ctrl = 7;
			if (z512_ctrl != 7)
				z512_present = 0;
			z512_ctrl = 0;
			if (z512_ctrl != 0)
				z512_present = 0;
		}
	}
	display_uarts();
	/* We can finally turn on the VDP interrupt if we are using it */
	if (timer_source == TIMER_TMS9918A)
		tms9918a_reload();
#ifdef CONFIG_DEV_I2C
	/* Fast systems will have a separate clock on the board */
	pcf8584_init(PCF_CLK_8MHZ);
#endif
}

void map_init(void)
{
}

uint8_t plt_param(unsigned char *p)
{
	/* If we have a keyboard then the display becomes a real tty
	   and we make it the primary console */
	if (strcmp(p, "zxkey") == 0 && !zxkey_present && !ps2kbd_present) {
		zxkey_present = 1;
		zxkey_init();
		vdu_setup();
		return 1;
	}
	return 0;
}

void device_init(void)
{
	if (eipc_present)
		ide_port = 0x0090;
#ifdef CONFIG_TD_IDE
#ifdef CONFIG_TD_PPIDE
	ppide_init();
#endif
	ide_probe();
#endif
#ifdef CONFIG_TD_SD
	pio_setup();
	kputs("sd0: ");
	sd_probe();
#endif
#ifdef CONFIG_CH375
	ch375_probe();
#endif
#ifdef CONFIG_TD_SCSI
	scsi_init();
#endif	
	lpr_init();
#ifdef CONFIG_NET
	sock_init();
#endif
}

/* Until we tidy the conditional build of floppy up provide a dummy symbol: FIXME */

#ifndef CONFIG_FLOPPY
uint8_t devfd_dtbl;
#endif
