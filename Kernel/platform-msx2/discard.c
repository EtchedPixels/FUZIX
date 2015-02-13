#include <kernel.h>
#include <kdata.h>
#include <devsd.h>
#include <printf.h>
#include "msx2.h"

extern int megasd_probe();

void device_init(void)
{
#ifdef CONFIG_RTC
    inittod();
#endif

    kprintf ("Running on a ");
    if (machine_type == MACHINE_MSX1) {
	kprintf("MSX1 not supported\n");
	// hang!
    } else if (machine_type == MACHINE_MSX2) {
	kprintf("MSX2 ");
    } else if (machine_type == MACHINE_MSX2P) {
        kprintf("MSX2+ ");
    } else if (machine_type == MACHINE_MSXTR) {
	kprintf("MSX TurboR ");
    }

    if ((infobits & KBDTYPE_MASK) == KBDTYPE_JPN) {
	kprintf("JP ");
    } else {
	kprintf("INT ");
    }
    if ((infobits & INTFREQ_MASK) == INTFREQ_60Hz) {
	kprintf("60Hz\n");
	ticks_per_dsecond = 6;
    } else {
	kprintf("50Hz\n");
	ticks_per_dsecond = 5;
    }

    if (megasd_probe()) {
        /* probe for megaflash rom sd */
        devsd_init();
    }
}

void map_init(void)
{
}


