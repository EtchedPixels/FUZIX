#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinydisk.h>
#include <tinysd.h>
#include <2063.h>

void map_init(void)
{
}

__sfr __at 0x81 vdp_c;

static void nap(void)
{
}

static uint8_t probe_tms9918a(void)
{
	uint16_t ct = 0;
	uint8_t v;

	/* Should clear top bit */
	v = vdp_c;

	/* Should see the top bit go high */
	do {
		v = vdp_c & 0x80;
	} while(--ct && !(v & 0x80));

	if (ct == 0)
		return 0;

	nap();

	/* Reading the F bit should have cleared it */
	if (vdp_c & 0x80)
		return 0;
	return 1;
}

/* 0x00 is the kernel, 0xF0 is the top 32K */
void pagemap_init(void)
{
	uint8_t i;
	for (i = 0x10; i < 0xF0; i += 0x10)
		pagemap_add(i);
	if (!probe_tms9918a())
		vdptype = 0xFF;	/* No VDP */
}

void device_init(void)
{
	uint8_t r = sd_init(0);
	if (r == 0)
		return;
	kputs("sd0: ");
	sd_shift[0] = (r & CT_BLOCK) ? 0 : 9;
	td_register(0, sd_xfer, td_ioctl_none, 1);
}
