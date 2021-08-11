#include <kernel.h>
#include "tm4c129x.h"

#include "gpio.h"

/*
 *  Chapter 10
 */

static const uint32_t bases[] = {
  0x4000U,  /* A */
  0x5000U,  /* B */
  0x6000U,  /* C */
  0x7000U,  /* D */
  0x24000U, /* E */
  0x25000U, /* F */
  0x26000U, /* G */
  0x27000U, /* H */
  0U,
  0x3d000U, /* J */
  0x61000U, /* K (AHB) */
  0x62000U, /* L (AHB) */
  0x63000U, /* M (AHB) */
  0x64000U, /* N (AHB) */
  0U,
  0x65000U, /* P (AHB) */
  0x66000U, /* Q (AHB) */
  0x67000U, /* R (AHB) */
  0x68000U, /* S (AHB) */
  0x69000U  /* T (AHB) */
};

static const unsigned shifts[] = {
  0U,  /* A */
  1U,  /* B */
  2U,  /* C */
  3U,  /* D */
  4U,  /* E */
  5U,  /* F */
  6U,  /* G */
  7U,  /* H */
  0U,
  8U,  /* J */
  9U,  /* K (AHB) */
  10U, /* L (AHB) */
  11U, /* M (AHB) */
  12U, /* N (AHB) */
  0U,
  13U, /* P (AHB) */
  14U, /* Q (AHB) */
  15U, /* R (AHB) */
  16U, /* S (AHB) */
  17U  /* T (AHB) */
};

#define GPIO_BASE(x) (0x40000000U + (bases[x]))
#define GPIO_SHIFT(x) (shifts[x])

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
#define GPIO_DR4R	0x0504
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

#define PCGCGPIO	(0x400FE000 + 0x0908)
#define RCGCGPIO	(0x400FE000 + 0x0608)

void gpio_write(unsigned int port, unsigned int pin, unsigned int onoff)
{
  outl(GPIO_BASE(port) + GPIO_DATA + (1U << (2U + pin)), onoff << pin);
}

static void gpio_power_up(unsigned int port)
{
  tm4c129x_modreg(PCGCGPIO, 0U, 1U << GPIO_SHIFT(port));
  tm4c129x_modreg(RCGCGPIO, 0U, 1U << GPIO_SHIFT(port));
}

static void gpio_func(unsigned int base,
                      uint32_t alt, uint32_t mask,
                      uint32_t amselclr, uint32_t amselset,
                      uint32_t afselclr, uint32_t afselset,
                      uint32_t dirclr, uint32_t dirset,
                      uint32_t odrclr, uint32_t odrset,
                      uint32_t denclr, uint32_t denset,
                      uint32_t purclr, uint32_t purset,
                      uint32_t pdrclr, uint32_t pdrset)
{
  uint32_t regval;

  if (dirset || dirclr)
    tm4c129x_modreg(base + GPIO_DIR, dirclr, dirset);
  if (afselset || afselclr)
    tm4c129x_modreg(base + GPIO_AFSEL, afselclr, afselset);
  if (odrset || odrclr)
    tm4c129x_modreg(base + GPIO_ODR, odrclr, odrset);
  if (purset || purclr)
    tm4c129x_modreg(base + GPIO_PUR, purclr, purset);
  if (pdrset || pdrclr)
    tm4c129x_modreg(base + GPIO_PDR, pdrclr, pdrset);
  if (denset || denclr)
    tm4c129x_modreg(base + GPIO_DEN, denclr, denset);
  if (pdrset || pdrclr)
    tm4c129x_modreg(base + GPIO_AMSEL, amselclr, amselset);
  regval = inl(base + GPIO_PCTL);
  regval &= ~mask;
  regval |= alt & mask;
  outl(base + GPIO_PCTL, regval);
}

#define GPIO_PMC_SHIFT(pin) ((pin) << 2U)
#define GPIO_PMC_VAL(pin, val) ((val) << (GPIO_PMC_SHIFT(pin)))
#define GPIO_PMC_MASK(pin) (GPIO_PMC_VAL((pin), 15U))

#define GPIO_FUNC_I(pin)                           \
  /* alt                  mask                  */ \
  0U,                     GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  1U << (pin),            0U,          /* AFSEL */ \
  1U << (pin),            0U,          /* DIR   */ \
  1U << (pin),            0U,          /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_O(pin)                           \
  /* alt                  mask                  */ \
  0U,                     GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  1U << (pin),            0U,          /* AFSEL */ \
  0U,                     1U << (pin), /* DIR   */ \
  1U << (pin),            0U,          /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_ODI(pin)                         \
  /* alt                  mask                  */ \
  0U,                     GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  1U << (pin),            0U,          /* AFSEL */ \
  1U << (pin),            0U,          /* DIR   */ \
  0U,                     1U << (pin), /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_ODO(pin)                         \
  /* alt                  mask                  */ \
  0U,                     GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  1U << (pin),            0U,          /* AFSEL */ \
  0U,                     1U << (pin), /* DIR   */ \
  0U,                     1U << (pin), /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_ODIO(pin, alt)                   \
  /* alt                  mask                  */ \
  GPIO_PMC_VAL(pin, alt), GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  0U,                     1U << (pin), /* AFSEL */ \
  0U,                     0U,          /* DIR   */ \
  0U,                     1U << (pin), /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_PFIO(pin, alt)                   \
  /* alt                  mask                  */ \
  GPIO_PMC_VAL(pin, alt), GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  1U << (pin),            0U,          /* AMSEL */ \
  0U,                     1U << (pin), /* AFSEL */ \
  0U,                     0U,          /* DIR   */ \
  1U << (pin),            0U,          /* ODR   */ \
  0U,                     1U << (pin), /* DEN   */ \
  0U,                     0U,          /* PUR   */ \
  0U,                     0U           /* PDR   */

#define GPIO_FUNC_AINPUT(pin)                      \
  /* alt                  mask                  */ \
  0U,                     GPIO_PMC_MASK(pin),      \
  /* clear                set                   */ \
  0U,                     1U << (pin), /* AMSEL */ \
  1U << (pin),            0U,          /* AFSEL */ \
  1U << (pin),            0U,          /* DIR   */ \
  1U << (pin),            0U,          /* ODR   */ \
  1U << (pin),            0U,          /* DEN   */ \
  1U << (pin),            0U,          /* PUR   */ \
  1U << (pin),            0U           /* PDR   */

static void gpio_pad(unsigned int base, unsigned int pin, unsigned int pad,
                     unsigned int strength)
{
  switch (strength) {
  case 2U:
    tm4c129x_modreg(base + GPIO_DR2R, 0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_DR4R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DR8R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_SLR,  1U << pin, 0U       );
    break;
  case 4U:
    tm4c129x_modreg(base + GPIO_DR2R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DR4R, 0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_DR8R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_SLR,  1U << pin, 0U       );
    break;
  case 8U:
    tm4c129x_modreg(base + GPIO_DR2R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DR4R, 1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DR8R, 0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_SLR,  1U << pin, 0U       );
    break;
  default:
    tm4c129x_modreg(base + GPIO_DR2R, 0U, 0U);
    tm4c129x_modreg(base + GPIO_DR4R, 0U, 0U);
    tm4c129x_modreg(base + GPIO_DR8R, 0U, 0U);
    tm4c129x_modreg(base + GPIO_SLR,  0U, 0U);
  }
  switch (pad) {
  case GPIO_PAD_STD:
    tm4c129x_modreg(base + GPIO_ODR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PUR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PDR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DEN,   0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_STDWPU:
    tm4c129x_modreg(base + GPIO_ODR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PUR,   0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_PDR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DEN,   0U,        1U << pin);
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_STDWPD:
    tm4c129x_modreg(base + GPIO_ODR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PUR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PDR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_DEN,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_OD:
    tm4c129x_modreg(base + GPIO_ODR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_PUR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PDR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DEN,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_ODWPU:
    tm4c129x_modreg(base + GPIO_ODR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_PUR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_PDR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DEN,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_ODWPD:
    tm4c129x_modreg(base + GPIO_ODR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_PUR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PDR,   0U       , 1U << pin);
    tm4c129x_modreg(base + GPIO_DEN,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_AMSEL, 1U << pin, 0U       );
    break;
  case GPIO_PAD_ANALOG:
    tm4c129x_modreg(base + GPIO_ODR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PUR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_PDR,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_DEN,   1U << pin, 0U       );
    tm4c129x_modreg(base + GPIO_AMSEL, 0U       , 1U << pin);
    break;
  default:
    tm4c129x_modreg(base + GPIO_ODR,   0U, 0U);
    tm4c129x_modreg(base + GPIO_PUR,   0U, 0U);
    tm4c129x_modreg(base + GPIO_PDR,   0U, 0U);
    tm4c129x_modreg(base + GPIO_DEN,   0U, 0U);
    tm4c129x_modreg(base + GPIO_AMSEL, 0U, 0U);
  }
}

void gpio_setup_pin(unsigned int port, unsigned int func, unsigned int pin,
                    unsigned int pad, unsigned int strength,
                    unsigned int alt, unsigned int value)
{
  uint32_t base = GPIO_BASE(port);

  gpio_power_up(port);
  gpio_func(base, GPIO_FUNC_I(pin));
  gpio_pad(base, pin, pad, strength);
  switch (func) {
  case GPIO_PINFUN_I:
    gpio_func(base, GPIO_FUNC_I(pin));
    break;
  case GPIO_PINFUN_O:
    gpio_func(base, GPIO_FUNC_O(pin));
    gpio_write(port, pin, value);
    break;
  case GPIO_PINFUN_ODI:
    gpio_func(base, GPIO_FUNC_ODI(pin));
    break;
  case GPIO_PINFUN_ODO:
    gpio_func(base, GPIO_FUNC_ODO(pin));
    gpio_write(port, pin, value);
    break;
  case GPIO_PINFUN_ODIO:
    gpio_func(base, GPIO_FUNC_ODIO(pin, alt));
    break;
  case GPIO_PINFUN_PFIO:
    gpio_func(base, GPIO_FUNC_PFIO(pin, alt));
    gpio_write(port, pin, value);
    break;
  case GPIO_PINFUN_AIO:
    gpio_func(base, GPIO_FUNC_AINPUT(pin));
    break;
  default:
    ;
  }
}
