#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devide.h>
#include <devatom.h>
#include <msm6242b.h>

/* FIXME */
extern int strcmp(const char *, const char *);

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

uint8_t plt_param(char *p)
{
	if (strcmp(p, "rtc") == 0) {
		samrtc = 1;
		return 1;
	}
	return 0;
}

void device_init(void)
{
#ifdef CONFIG_RTC
	/* Time of day clock */
	plt_rtc_probe();
	inittod();
#endif
	if (!mouse_probe())
		mouse_present = 1;
	if (atom_probe())
		devide_init();
}
