#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <hardware/tiva_memorymap.h>
#include <hardware/tiva_pinmap.h>
#include <hardware/tiva_ssi.h>
#include <arch/board/board.h>
#include <tiva_enablepwr.h>
#include <tiva_enableclks.h>
#include <tiva_gpio.h>
#include <tm4c_gpio.h>

#include "cpu.h"

#include "ssi.h"

static struct {
  uintptr_t base;
  uint32_t modebits;
  size_t nbits;
  struct {
    uint32_t requested;
    uint32_t actual;
  } frequency;
  const void *txbuffer;
  void *rxbuffer;
  size_t ntxwords;
  size_t nrxwords;
  size_t nwords;
} ssi_ports[] = {
  { .base = TIVA_SSI0_BASE,
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = TIVA_SSI1_BASE,
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = TIVA_SSI2_BASE,
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = TIVA_SSI3_BASE,
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
};

static inline uint32_t ssi_getreg(int port, unsigned int offset)
{
  return getreg32(ssi_ports[port].base + offset);
}

static inline void ssi_putreg(int port, unsigned int offset, uint32_t value)
{
  putreg32(value, ssi_ports[port].base + offset);
}

static uint32_t ssi_disable(int port)
{
  uint32_t regval;
  uint32_t retval = ssi_getreg(port, TIVA_SSI_CR1_OFFSET);

  regval = (retval & (~SSI_CR1_SSE));
  ssi_putreg(port, TIVA_SSI_CR1_OFFSET, regval);
  return retval;
}

static void ssi_enable(int port, uint32_t enable)
{
  uint32_t regval = ssi_getreg(port, TIVA_SSI_CR1_OFFSET);

  regval &= (~SSI_CR1_SSE);
  regval |= (enable & SSI_CR1_SSE);
  ssi_putreg(port, TIVA_SSI_CR1_OFFSET, regval);
}

static void ssi_setmodebits_unlocked(int port, uint32_t modebits)
{
  uint32_t regval;

  if (modebits != (ssi_ports[port].modebits)) {
    regval  = ssi_getreg(port, TIVA_SSI_CR0_OFFSET);
    regval &= (~(SSI_CR0_FRF_MASK | SSI_CR0_SPH | SSI_CR0_SPO));
    regval |= modebits;
    ssi_putreg(port, TIVA_SSI_CR0_OFFSET, regval);
    ssi_ports[port].modebits = modebits;
  }
}

static void ssi_setnbits_unlocked(int port, size_t nbits)
{
  uint32_t regval;

  if ((nbits != (ssi_ports[port].nbits)) && (nbits >= 4U) && (nbits <= 16U)) {
    regval  = ssi_getreg(port, TIVA_SSI_CR0_OFFSET);
    regval &= (~SSI_CR0_DSS_MASK);
    regval |= ((nbits - 1U) << SSI_CR0_DSS_SHIFT);
    ssi_putreg(port, TIVA_SSI_CR0_OFFSET, regval);
    ssi_ports[port].nbits = nbits;
  }
}

static uint32_t ssi_setfrequency_unlocked(int port, uint32_t frequency)
{
  uint32_t maxdvsr;
  uint32_t cpsdvsr;
  uint32_t regval;
  uint32_t scr = 0U;

  if (frequency != (ssi_ports[port].frequency.requested)) {
    if (frequency > (SYSCLK_FREQUENCY / 2U))
      frequency = (SYSCLK_FREQUENCY / 2U);
    maxdvsr = (SYSCLK_FREQUENCY / frequency);
    cpsdvsr = 0U;
    do {
      cpsdvsr += 2U;
      scr = ((maxdvsr / cpsdvsr) - 1U);
    } while (scr > 255U);
    ssi_putreg(port, TIVA_SSI_CPSR_OFFSET, cpsdvsr);
    regval = ssi_getreg(port, TIVA_SSI_CR0_OFFSET);
    regval &= (~SSI_CR0_SCR_MASK);
    regval |= (scr << SSI_CR0_SCR_SHIFT);
    ssi_putreg(port, TIVA_SSI_CR0_OFFSET, regval);
    ssi_ports[port].frequency.requested = frequency;
    ssi_ports[port].frequency.actual = (SYSCLK_FREQUENCY /
                                        (cpsdvsr * (scr + 1U)));
  }
  return ssi_ports[port].frequency.actual;
}

uint32_t ssi_setfrequency(int port, uint32_t frequency)
{
  uint32_t retval;
  uint32_t enabled = ssi_disable(port);

  retval = ssi_setfrequency_unlocked(port, frequency);
  ssi_enable(port, enabled);
  return retval;
}

static inline bool ssi_rxfifoempty(int port)
{
  return 0U == (ssi_getreg(port, TIVA_SSI_SR_OFFSET) & SSI_SR_RNE);
}

static inline bool ssi_txfifofull(int port)
{
  return 0U == (ssi_getreg(port, TIVA_SSI_SR_OFFSET) & SSI_SR_TNF);
}

void ssi_txnull(int port)
{
  ssi_putreg(port, TIVA_SSI_DR_OFFSET, 0xffffU);
}

static void ssi_txuint16(int port)
{
  const uint16_t *ptr = ((const uint16_t *)(ssi_ports[port].txbuffer));

  ssi_putreg(port, TIVA_SSI_DR_OFFSET, (uint32_t)(*ptr++));
  ssi_ports[port].txbuffer = ptr;
}

static void ssi_txuint8(int port)
{
  const uint8_t *ptr = ((const uint8_t *)(ssi_ports[port].txbuffer));

  ssi_putreg(port, TIVA_SSI_DR_OFFSET, ((uint32_t)(*ptr++)));
  ssi_ports[port].txbuffer = ptr;
}

void ssi_rxnull(int port)
{
  ssi_getreg(port, TIVA_SSI_DR_OFFSET);
}

static void ssi_rxuint16(int port)
{
  uint16_t *ptr = ((uint16_t *)(ssi_ports[port].rxbuffer));

  *ptr = ((uint16_t)(ssi_getreg(port, TIVA_SSI_DR_OFFSET)));
  ssi_ports[port].rxbuffer = (++ptr);
}

static void ssi_rxuint8(int port)
{
  uint8_t *ptr = ((uint8_t *)(ssi_ports[port].rxbuffer));

  *ptr = ((uint8_t)(ssi_getreg(port, TIVA_SSI_DR_OFFSET)));
  ssi_ports[port].rxbuffer = (++ptr);
}

static int ssi_performtx(int port, void (*txword)(int))
{
  unsigned ntxd = 0U;

  if (!(ssi_txfifofull(port))) {
    if ((ssi_ports[port].ntxwords) > 0U) {
      for (; (ntxd < (ssi_ports[port].ntxwords)) && (!(ssi_txfifofull(port)));
           ntxd++)
        txword(port);
      ssi_ports[port].ntxwords -= ntxd;
    }
  }
  return ntxd;
}

static inline void ssi_performrx(int port, void (*rxword)(int))
{
  while (!(ssi_rxfifoempty(port))) {
    if ((ssi_ports[port].nrxwords) < (ssi_ports[port].nwords)) {
      rxword(port);
      ssi_ports[port].nrxwords++;
    }
  }
}

void ssi_transfer(int port, const void *txbuffer, void *rxbuffer, size_t nwords)
{
  void (*txword)(int) = NULL;
  void (*rxword)(int) = NULL;

  ssi_ports[port].txbuffer = txbuffer;
  ssi_ports[port].rxbuffer = rxbuffer;
  ssi_ports[port].ntxwords = nwords;
  ssi_ports[port].nrxwords = 0U;
  ssi_ports[port].nwords   = nwords;
  if ((ssi_ports[port].nbits) > 8U) {
    txword = ssi_txuint16;
    rxword = ssi_rxuint16;
  } else {
    txword = ssi_txuint8;
    rxword = ssi_rxuint8;
  }
  if (!txbuffer)
    txword = ssi_txnull;
  if (!rxbuffer)
    rxword = ssi_rxnull;
  do {
    ssi_performtx(port, txword);
    ssi_performrx(port, rxword);
  } while ((ssi_ports[port].nrxwords) < (ssi_ports[port].nwords));
}

void ssi_init(int which)
{
  switch (which) {
  case 0:
    tiva_ssi0_enablepwr();
    tiva_ssi0_enableclk();
    break;
  case 1:
    tiva_ssi1_enablepwr();
    tiva_ssi1_enableclk();
    break;
  case 2:
    tiva_ssi2_enablepwr();
    tiva_ssi2_enableclk();
    break;
  case 3:
    tiva_ssi3_enablepwr();
    tiva_ssi3_enableclk();
    /* PQ0/SSI3CLK */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTQ | GPIO_PIN_0);
    /* PQ1/SSI3FSS */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTQ | GPIO_PIN_1);
    /* PQ2/SSI3XDAT0 */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTQ | GPIO_PIN_2);
    /* PF0/SSI3XDAT1 */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTF | GPIO_PIN_0);
    /* PF4/SSI3XDAT2 */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTF | GPIO_PIN_4);
    /* PF5/SSI3XDAT3 */
    tiva_configgpio(GPIO_FUNC_PFIO   | GPIO_ALT_14 | GPIO_PORTF | GPIO_PIN_5);
    /* PH4 */
    tiva_configgpio(GPIO_FUNC_OUTPUT               | GPIO_PORTH | GPIO_PIN_4);
    tiva_gpiowrite(GPIO_PORTQ | GPIO_PIN_1, true);
    tiva_gpiowrite(GPIO_PORTH | GPIO_PIN_4, true);
    break;
  default:
    for (;;)
      asm("wfe");
  }
  ssi_putreg(which, TIVA_SSI_CR1_OFFSET, 0U);
  ssi_putreg(which, TIVA_SSI_CR0_OFFSET, 0U);
  ssi_setmodebits_unlocked(which, 0U);
  ssi_setnbits_unlocked(which, 8U);
  ssi_setfrequency_unlocked(which, 400000U);
  ssi_putreg(which, TIVA_SSI_IM_OFFSET, 0U);
  ssi_enable(which, SSI_CR1_SSE);
}
