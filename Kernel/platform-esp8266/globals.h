#ifndef GLOBALS_H
#define GLOBALS_H

/* These reflect a stanard ESP8266 configuration */
#define CPU_CLOCK 160		/* We switch to the double clock */
#define PERIPHERAL_CLOCK 52	/* 26MHz crystal */

extern void flash_dev_init(void);

extern void fuzix_main(void);
extern void sd_rawinit(void);

#endif

/* vim: sw=4 ts=4 et: */

