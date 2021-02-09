#ifndef GLOBALS_H
#define GLOBALS_H

#define CPU_CLOCK 80
#define PERIPHERAL_CLOCK 80

extern void flash_dev_init(void);

extern void fuzix_main(void);
extern void sd_rawinit(void);

#endif

/* vim: sw=4 ts=4 et: */

