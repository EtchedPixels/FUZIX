#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <ds1302.h>
#include <devide.h>
#include <blkdev.h>
#include <ppide.h>
#include <rc2014.h>
#include "config.h"
#include "vfd-term.h"
#include "z180_uart.h"

/* Everything in here is discarded after init starts */

__sfr __at 0x99 tms9918a_ctrl;

static uint8_t probe_tms9918a(void)
{
	uint16_t ct = 0;
	/* Read the status port */
	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x8F;
	/* No fifth sprite collision */
	if (tms9918a_ctrl & 0x7F)
		return 0;
	/* Try turning it on and looking for a vblank */
	tms9918a_ctrl = 0x00;
	tms9918a_ctrl = 0x80;
	tms9918a_ctrl = 0x09;
	tms9918a_ctrl = 0x81;
	/* Should see the top bit go high */
	while(ct-- && (tms9918a_ctrl & 0x80));
	if (ct == 0)
		return 0;
	/* Reading the F bit should have cleared it */
	if (tms9918a_ctrl & 0x80)
		return 0;
	return 1;
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
				return UART_16750;
			return UART_16550A;
		}
		/* Should never find this real ones were discontinued
		   very early due to a hardware bug */
		return UART_16550;
	} else {
		/* Decode types without FIFO */
		out(p + 7, 0x2A);
		if (in (p + 7) == 0x2A)
			return UART_16450;
		return UART_8250;
	}
}

const char *uart_name[] = {
	"?",
	"6850 ACIA",
	"Zilog SIO",
	"Z180",
	"8250",
	"16450",
	"16550",
	"16550A",
	"16750"
};

void init_hardware_c(void)
{
#ifdef CONFIG_VFD_TERM
	vfd_term_init();
#endif
	ramsize = 512;
	procmem = 512 - 80;

	/* Set the right console for kernel messages */

	/* FIXME: When ROMWBW handles 16550A or second SIO, or Z180 as
	   console we will need to address this better */

	if (z180_present) {
		kputs("Z180 CPU card detected.\n");
		z180_setup(!ctc_present);
		register_uart(UART_Z180, Z180_IO_BASE, &z180_uart0);
		register_uart(UART_Z180, Z180_IO_BASE + 1, &z180_uart1);
	}

	if (acia_present) {
		register_uart(UART_ACIA, 0x80, &acia_uart);
	} else {
		register_uart(UART_SIO, 0x80, &sio_uart);
		register_uart(UART_SIO, 0x82, &sio_uartb);
	}
}

void pagemap_init(void)
{
	uint8_t i, m;

	/* RC2014 512/512K has RAM in the top 512 KiB of physical memory
	 * corresponding pages are 32-63 (page size is 16 KiB)
	 * Pages 32-34 are used by the kernel
	 * Page 35 is the common area for init
	 * Page 36 is the disk cache
	 * Pages 37 amd 38 are the second kernel bank
	 */
	for (i = 32 + 7; i < 64; i++)
		pagemap_add(i);

	/* finally add the common area */
	pagemap_add(32 + 3);

	/* The DS1302 clashes with the Z180 ports */
	if (!z180_present)
		ds1302_init();

	if (acia_present)
		kputs("6850 ACIA detected at 0x80.\n");
	if (sio_present)
		kputs("Z80 SIO detected at 0x80.\n");

	/* Further ports we register at this point */
	if (sio1_present) {
		kputs("Z80 SIO detected at 0x84.\n");
		register_uart(UART_SIO, 0x84, &sio_uart);
		register_uart(UART_SIO, 0x86, &sio_uartb);
	}
	if (ctc_present)
		kputs("Z80 CTC detected at 0x88.\n");

	/* Not clear how we should attach this to a console, for now
	   our tty driver logic attaches it to the first console port */
	tms9918a_present = probe_tms9918a();
	if (tms9918a_present)
		kputs("TMS9918A at 0x98/99.\n");

	/* Devices in the C0-CF range cannot be used with Z180 */
	if (!z180_present) {
		i = 0xC0;

		if (ds1302_present) {
			kputs("DS1302 detected at 0xC0.\n");
			/* UART at 0xC0 means no DS1302 there */
			i += 8;
		}
		while(i) {
			if ((m = probe_16x50(i)))
				register_uart(m, i, &ns16x50_uart);
			i += 0x08;
		}
	}
}

void map_init(void)
{
}

void device_init(void)
{
#ifdef CONFIG_IDE
	devide_init();
#ifdef CONFIG_PPIDE
	ppide_init();
#endif
#endif
}
