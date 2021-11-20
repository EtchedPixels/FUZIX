#ifndef GLOBALS_H
#define GLOBALS_H

struct exception_frame {
	/* Saved in the fault handler */
	uint32_t a0;
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;
	uint32_t a8;
	uint32_t a9;
	uint32_t a10;
	uint32_t a11;
	uint32_t spare;
	uint32_t a14;
	uint32_t a15;
	uint32_t sar;
	uint32_t epc1;
	uint32_t exccause;
	uint32_t excvaddr;
	/* The registers saved in the initial stub */
	uint32_t a12;
	uint32_t a13;
};

extern void fuzix_main(void);

extern void timer_init(void);
extern void irq_set_mask(uint32_t mask);
extern void irq_enable(uint32_t irq);
extern void irq_disable(uint32_t irq);

extern void __attribute__((noreturn)) appcore_main(void);
extern void __attribute__((noreturn)) procore_main(void);

#endif
