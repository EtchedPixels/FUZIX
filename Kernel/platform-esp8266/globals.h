#ifndef GLOBALS_H
#define GLOBALS_H

/* These reflect a stanard ESP8266 configuration */
#define CPU_CLOCK 160		/* We switch to the double clock */
#define PERIPHERAL_CLOCK 52	/* 26MHz crystal */

extern void flash_dev_init(void);

extern void fuzix_main(void);
extern void sd_rawinit(void);

extern void irq_set_mask(uint32_t mask);
extern void irq_enable(uint32_t irq);
extern void irq_disable(uint32_t irq);
#endif

/* vim: sw=4 ts=4 et: */

