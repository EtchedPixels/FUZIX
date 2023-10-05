#include <kernel.h>
#include <kdata.h>
#include <devsd.h>
#include <devide_sunrise.h>
#include <printf.h>
#include <vt.h>
#include <tty.h>
#include "msx2.h"
#include "devtty.h"
#include "kbdmatrix.h"

extern int megasd_probe(void);

void device_init(void)
{
#ifdef CONFIG_RTC
    inittod();
#endif

    if (machine_type == MACHINE_MSX1) {
	panic("MSX1 not supported\n");
    } else if (machine_type == MACHINE_MSX2) {
	kprintf("MSX2");
    } else if (machine_type == MACHINE_MSX2P) {
        kprintf("MSX2+");
    } else if (machine_type == MACHINE_MSXTR) {
	kprintf("MSX TurboR");
    }

    /* keyboard layout initialization: default is international,
     * localized variations overlay on top */
    memcpy(keyboard, keyboard_int, sizeof(keyboard_int));
    memcpy(shiftkeyboard, shiftkeyboard_int, sizeof(shiftkeyboard_int));

    if ((infobits & KBDTYPE_MASK) == KBDTYPE_JPN) {
	kprintf("(JPN)");
	memcpy(keyboard, keyboard_jp, sizeof(keyboard_jp));
	memcpy(shiftkeyboard, shiftkeyboard_jp, sizeof(shiftkeyboard_jp));
    } else if ((infobits & KBDTYPE_MASK) == KBDTYPE_UK) {
	kprintf("(UK)");
	memcpy(&shiftkeyboard[2][0],shiftkeyboard_uk, sizeof(shiftkeyboard_uk));
    } else if ((infobits & KBDTYPE_MASK) == KBDTYPE_ES) {
	kprintf("(ES)");
	memcpy(&keyboard[1][0], keyboard_es, sizeof(keyboard_es));
	memcpy(&shiftkeyboard[1][0], shiftkeyboard_es, sizeof(shiftkeyboard_es));
    }
    kputs(" with ");
    if ((infobits & INTFREQ_MASK) == INTFREQ_60Hz) {
	kputs("NTSC ");
	ticks_per_dsecond = 6;
    } else {
	kputs("PAL ");
    }
    kputs("video.\n");

    /* Default key repeat values in 10ths of seconds */
    /* These are the 50/60 Hz keyrepeat values observed for MSX2 and above */
    keyrepeat.first = ticks_per_dsecond == 6 ? 40 : 32;
    keyrepeat.continual = 2;

    sunrise_probe();
    if (megasd_probe()) {
        /* probe for megaflash rom sd */
        devsd_init();
    }
}

void map_init(void)
{
}
