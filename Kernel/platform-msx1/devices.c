#include <kernel.h>
#include <printf.h>
#include <version.h>
#include <kdata.h>
#include <devices.h>
#include <devide_sunrise.h>
#include <devfd.h>
#include <devlpr.h>
#include <devsys.h>
#include <vt.h>
#include <tty.h>
#include <keycode.h>
#include <devtty.h>
#include <msx.h>
#include <kbdmatrix.h>
#include <blkdev.h>
#include <devide_sunrise.h>

struct devsw dev_tab[] =  /* The device driver switch table */
{
  /* 0: /dev/hd		Hard disc block devices (and RAM etc) */
  {  blkdev_open,     no_close,    blkdev_read,   blkdev_write,   no_ioctl },
  /* 1: /dev/fd		Floppy disc block devices */
  {  fd_open,     no_close,    fd_read,   fd_write,   no_ioctl },
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  vt_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  lpr_open,     no_close,   no_rdwr,   lpr_write,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
};

bool validdev(uint16_t dev)
{
    /* This is a bit uglier than needed but the right hand side is
       a constant this way */
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

/*
 *	We may want to do this differently in the end but report the
 *	cartridge hashes found.
 *
 *	A4B6 = Sony HBD-F1
 *	3B49 = Sony HBK-30
 */
void device_init(void)
{
#if 0
  int i;
  for (i = 0; i < 16; i++) {
    if (slot_table[i])
      kprintf("%d.%d: %x\n", (i>>2) & 3, i & 3, slot_table[i]);
  }
#endif
#ifdef CONFIG_RTC
    inittod();
#endif

    kprintf ("Running on a ");
    if (machine_type == MACHINE_MSX1) {
	kprintf("MSX1\n");
    } else if (machine_type == MACHINE_MSX2) {
	kprintf("MSX2 ");
    } else if (machine_type == MACHINE_MSX2P) {
        kprintf("MSX2+ ");
    } else if (machine_type == MACHINE_MSXTR) {
	kprintf("MSX TurboR ");
    }

    /* keyboard layout initialization: default is international,
     * localized variations overlay on top */
    memcpy(keyboard, keyboard_int, sizeof(keyboard_int));
    memcpy(shiftkeyboard, shiftkeyboard_int, sizeof(shiftkeyboard_int));

    if ((infobits & KBDTYPE_MASK) == KBDTYPE_JPN) {
	kprintf("JPN ");
	memcpy(keyboard, keyboard_jp, sizeof(keyboard_jp));
	memcpy(shiftkeyboard, shiftkeyboard_jp, sizeof(shiftkeyboard_jp));
    } else if ((infobits & KBDTYPE_MASK) == KBDTYPE_UK) {
	kprintf("UK ");
	memcpy(&shiftkeyboard[2][0],shiftkeyboard_uk, sizeof(shiftkeyboard_uk));
    } else if ((infobits & KBDTYPE_MASK) == KBDTYPE_ES) {
	kprintf("ES ");
	memcpy(&keyboard[1][0], keyboard_es, sizeof(keyboard_es));
	memcpy(&shiftkeyboard[1][0], shiftkeyboard_es, sizeof(shiftkeyboard_es));
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

    /* Default key repeat values in 10ths of seconds */
    keyrepeat.first = 2 * ticks_per_dsecond;
    keyrepeat.continual = 1 * ticks_per_dsecond;

    sunrise_probe();
}
