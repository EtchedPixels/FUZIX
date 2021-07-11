#include <kernel.h>
#include "tm4c129x.h"

/*
 *	Chapter 10
 */

#define GPIO_BASE(x)	(0x40058000 + ((x) * 0x1000))

#define GPIO_DATA	0x0000
#define GPIO_DIR	0x0400
#define GPIO_IS		0x0404
#define GPIO_IBE	0x0408
#define GPIO_IEV	0x040C
#define GPIO_IM		0x0410
#define GPIO_RIS	0x0414
#define GPIO_MIS	0x0418
#define GPIO_ICR	0x041C
#define GPIO_AFSEL	0x0420
#define GPIO_DR2R	0x0500
#define GPIO_DR4DR	0x0504
#define GPIO_DR8R	0x0508
#define GPIO_ODR	0x050C
#define GPIO_PUR	0x0510
#define GPIO_PDR	0x0514
#define GPIO_SLR	0x0518
#define GPIO_DEN	0x051C
#define GPIO_LOCK	0x0520
#define GPIO_CR		0x0524
#define GPIO_AMSEL	0x0528
#define GPIO_PCTL	0x052C
#define GPIO_ADCCTL	0x0530
#define GPIO_DMACTL	0x0534
#define GPIO_SI		0x0538
#define GPIO_DR12R	0x053C
#define GPIO_WAKEPEN	0x0540
#define GPIO_WAKELVL	0x0544
#define GPIO_WAKESTAT	0x0588
#define GPIO_PP		0x0FC0
#define GPIO_PC		0x0FC4

#define RCGCGPIO	(0x400FE000 + 0x0608)
#define PCGCGPIO	(0x400FE000 + 0x0908)

void gpio_write(unsigned int port, unsigned int pin, unsigned int onoff)
{
    uint32_t mask;
    uint32_t base = GPIO_BASE(port);
       
    /* There are 256 incarnations of the data port on dword boundaries. The
       address bits for each (ie 9-2) control which bits are affected. */
    mask = 4 << pin;
    outl(base + mask + GPIO_DATA, onoff << pin);
}

static void gpio_power_up(unsigned int port)
{
    outmod32(PCGCGPIO, 1 << port, 1);
    outmod32(RCGCGPIO, 1 << port, 1);
}

/* Set a GPIO up as a normal output */
void gpio_output(unsigned int port, unsigned int pin)
{
    uint32_t base = GPIO_BASE(port);

    gpio_power_up(port);
   
    /* Output */
    outmod32(base + GPIO_DIR, 1 << pin, 1 << pin);
    /* Select GPIO mode */
    outmod32(base + GPIO_AFSEL, 1 << pin, 0);
    /* Not open drain */
    outmod32(base + GPIO_ODR, 1 << pin, 0);
    /* No pull up */
    outmod32(base + GPIO_PUR, 1 << pin, 0);
    /* No pull down */
    outmod32(base + GPIO_PDR, 1 << pin, 0);
    /* Enable digital output */
    outmod32(base + GPIO_DEN, 1 << pin, 1 << pin);
    /* Disable analogue circuitry */
    outmod32(base + GPIO_AMSEL, 1 << pin, 0);
}

/* Hand a GPIO over to a special function */
void gpio_altfunc(unsigned int port, unsigned int pin, unsigned int func)
{
    uint32_t base = GPIO_BASE(port);
    uint32_t r;

    gpio_power_up(port);
   
    /* Output */
    outmod32(base + GPIO_DIR, 1 << pin, 1 << pin);
    /* Select GPIO mode */
    outmod32(base + GPIO_AFSEL, 1 << pin, 1 << pin);
    /* Not open drain */
    outmod32(base + GPIO_ODR, 1 << pin, 0);
    /* No pull up */
    outmod32(base + GPIO_PUR, 1 << pin, 0);
    /* No pull down */
    outmod32(base + GPIO_PDR, 1 << pin, 0);
    /* Enable digital output */
    outmod32(base + GPIO_DEN, 1 << pin, 1 << pin);
    /* Disable analogue circuitry */
    outmod32(base + GPIO_AMSEL, 1 << pin, 0);
    /* Set the alternate function */
    r = inl(base + GPIO_PCTL);
    r &= ~0x0F << (4 * pin);
    r |= func << (4 * pin);
    outl(base + GPIO_PCTL, r);
}
