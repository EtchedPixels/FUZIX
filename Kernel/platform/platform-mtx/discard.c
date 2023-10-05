#include <kernel.h>
#include <printf.h>
#include <blkdev.h>
#include <mtx.h>

extern uint8_t membanks;

static const char *vdpnametab[] = {
  "TMS9929A",
  "V9938",
  "V9958"
};

__sfr __at 0xFF turbo;

void pagemap_init(void)
{
	const char *vdpname = "??";
	extern uint8_t kernel_map;
	int i;
	/* Up to 16 banks */
	kprintf("%d memory banks detected.\n", membanks);
	if (membanks < 3)
		panic("Insufficient memory.\n");
	kprintf("Kernel in bank %d.\n", kernel_map & 0x7F);

	for (i = 0x80; i < 0x80 + membanks; i++)
		if (i != kernel_map)
			pagemap_add(i);

	if (vdptype < 3)
		vdpname = vdpnametab[vdptype];
	kprintf("VDP: %s\n", vdpname);
	if (vdptype == 2) {
		has_mtxplus = 1;
		kputs("Engaging MTXplus turbo.\n");
		turbo = 0x70;
	}
}

uint8_t plt_param(char *p)
{
	used(p);
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
}
