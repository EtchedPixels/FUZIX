#ifndef GLOBALS_H
#define GLOBALS_H

extern void flash_dev_init(void);

extern void fuzix_main(void);
extern void sd_rawinit(void);

extern void timer_init(void);
extern void irq_set_mask(uint32_t mask);
extern void irq_enable(uint32_t irq);
extern void irq_disable(uint32_t irq);

#endif

/* vim: sw=4 ts=4 et: */

