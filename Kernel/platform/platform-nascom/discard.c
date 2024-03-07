#include <kernel.h>
#include <devtty.h>
#include <devscsi.h>
#include <printf.h>
#include <blkdev.h>
#include <tty.h>
#include <nascom.h>
#include <fdc80.h>

uint8_t io_page;
uint16_t bankmap;

static void probe_mm58174(void)
{
	register uint8_t n;
	register unsigned ct = 0;

	kputs("Probing mm58174 @0x20\n");
	if ((in(clk_tsecs) & 0x0F) > 5)
		return;
	if ((in(clk_tmins) & 0x0F) > 5)
		return;
	if ((in(clk_thours) & 0x0F) > 2)
		return;
	if ((in(clk_tdays) & 0x0F) > 3)
		return;
	if ((in(clk_tmons) & 0x0F) > 1)
		return;
	/* Looks plausible */
	n = in(clk_tenths) + 1;
	if (n == 10)
		n = 0;
	while(++ct) {
		if (n == in(clk_tenths)) {
			kputs("mm58174 detected @0x20\n");
			have_mm58174 = 1;
			out(clk_stat, 0x0);
			in(clk_stat);
			in(clk_stat);
			in(clk_stat);	/* Reset */
			break;
		}
	}
	/* If it was an rtc it's not ticking */
}

void device_init(void)
{
	register unsigned i;
	/* TODO: rtc init proper */
	kputs("Checking for IDE interface on PIO\n");
	ide_pio_setup();
	ide_probe();
	/* Check for a RAMdisc */
	if (gm833_probe() && swap_dev == 0xFFFF) {
		/* If we found no swap use ramdisc 0 as 512K of swap */
		swap_dev = 0x0800;
		for (i = 0; i < 10; i++)
			swapmap_add(i * 48);
		kputs("Using /dev/rd0 GM833 for swap\n");
	}
	/* Check for MM58174 */
	probe_mm58174();
	/* And floppies */
	if (fdc80_probe() == FDC_GM849)	/* 849 also check the SASI */
		gm849_sasi_init();
}

void map_init(void)
{
}

/* Add pagemap codes depending upon the present banks */
void pagemap_init(void)
{
#ifdef CONFIG_MAP80
	register unsigned m = bankmap;
	register unsigned n = 30;
	if ((!bankmap & 0x7FFF))	/* No extra pages ? */
		panic("no map80 banks");
	while(n) {
		if (m & 1)
			pagemap_add(n);
		n -= 2;
		m >>= 1;
	}
#else
	/* We didn't find any extra page mode memory */
	if (bankmap < (2 << 8))
		panic("no pagemode");
	/* Add the present banks (may be non contiguous). The
	   first bank is the kernel and not available */
	if (bankmap & 2)
		pagemap_add(0x22);
	if (bankmap & 4)
		pagemap_add(0x44);
	if (bankmap & 8)
		pagemap_add(0x88);
#endif
}
