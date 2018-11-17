#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devide.h>
#include <devatom.h>
#include <msm6242b.h>

void map_init(void)
{
}

/* Pages 0/1/2/3 are the kernel 4/5 are display and fonts */
void pagemap_init(void)
{
	uint8_t maxpages;
	uint8_t i;

	maxpages = ramsize / 16;

	for (i = 6; i < maxpages; i += 2)
		pagemap_add(i);
}

uint8_t platform_param(char *p)
{
	used(p);
	return 0;
}

void device_init(void)
{
#ifdef CONFIG_RTC
	/* Time of day clock */
	platform_rtc_probe();
	inittod();
#endif
	if (atom_probe())
		devide_init();
}
