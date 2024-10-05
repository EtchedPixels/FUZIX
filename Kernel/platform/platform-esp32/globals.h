#ifndef GLOBALS_H
#define GLOBALS_H

extern void fuzix_main(void);

extern void timer_init(void);
extern void irq_set_mask(uint32_t mask);
extern void irq_enable(uint32_t irq);
extern void irq_disable(uint32_t irq);

extern void __attribute__((noreturn)) appcore_main(void);
extern void __attribute__((noreturn)) procore_main(void);

#endif
