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

/* 0x00 is the kernel, 0xF0 is the top 32K */
void pagemap_init(void)
{
	uint8_t i;
	for (i = 0x10; i < 0xF0; i += 0x10)
		pagemap_add(i);
}

void device_init(void)
{
	kputs("SD");
	uint8_t r = sd_init();
	if (r == 0)
		return;
	kputs("sd0: ");
	sd_shift[0] = (r & CT_BLOCK) ? 0 : 9;
	td_register(sd_xfer, 1);
}
