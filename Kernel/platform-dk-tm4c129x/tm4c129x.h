#ifndef __TM4C129X_H
#define __TM4C129X_H

#include <stdint.h>

#define SDCARD_SSI_PORT 3

void tm4c129x_init(void);
int sysclock_init(void);

/*
 *	Hardware definitions
 */

#define SYS_CLOCK			120000000	/* 120MHz */

/*
 *	NVIC interrupt controller
 */
#define NVIC_ICTR			(0xE000E004)
#define NVIC_SYSTICK_CTRL		(0xE000E010)
#define NVIC_SYSTICK_CTRL_ENABLE	0x01
#define NVIC_SYSTICK_CTRL_TICKINT	0x02
#define NVIC_SYSTICK_CTRL_CLKSOURCE	0x04
#define NVIC_SYSTICK_CTRL_COUNTFLAG	0x10000

#define NVIC_SYSTICK_RELOAD		(0xE000E014)
#define NVIC_SYSTICK_CURRENT_OFFSET	(0xE000E018)
#define NVIC_SYSTICK_CALIB_OFFSET	(0xE000E01C)

#define NVIC_VECTAB			(0xE000ED08)
#define NVIC_ENABLE			0xE000E100
#define NVIC_CLRENABLE			0xE000E180
#define NVIC_IRQ0_31_CLEAR		0xE000E180
#define NVIC_ICTR_INTLINESNUM_MASK	0x0F
#define NVIC_SYSH_PRIORITY_PR15_MASK	0xFF000000
#define NVIC_SYSH_PRIORITY_PR15_SHIFT	24

#define NVIC_SYSH12_15_PRIORITY		(0xE000ED20)

#define NVIC_SYSH_PRIORITY_DEFAULT	0x80

#define NVIC_SYSHCON			(0xE000ED24)
#define NVIC_SYSHCON_MEMFAULTEN		0x10000
#define NVIC_SYSHCON_BUSFAULTEN		0x20000
#define NVIC_SYSHCON_USGFAULTEN		0x40000

/*
 *	GPIO
 */

void gpio_write(unsigned int port, unsigned int pin, unsigned int onoff);
void gpio_output(unsigned int port, unsigned int pin);
void gpio_altfunc(unsigned int port, unsigned int pin, unsigned int func);

/* Ports are named A B etc in the docs so this is easier to read. Mask so
   lower case doesn't produce weird bugs */
#define GPIO_PORT(n)	(((n) & ~0x20) - 'A')


#endif /* __TM4C129X_H */
