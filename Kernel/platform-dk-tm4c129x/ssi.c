#include <kernel.h>
#include "tm4c129x.h"
#include "gpio.h"
#include "ssi.h"

/*
 *	Chapter 20: QSSI
 */

#define SSI_BASE(n)	(0x40008000 + 0x1000 * (n))

#define SSI_CR0		0x0000
#define		CR0_SPH	0x80
#define		CR0_SPO	0x40
#define		CR0_FRF	0x30
#define		CR0_DSS 0x0F
#define		CR0_SCR 0xFF00
#define		CR0_SCR_SHIFT 8U

#define SSI_CR1		0x0004
#define		CR1_SSE	0x02
#define SSI_DR		0x0008
#define SSI_SR		0x000C
#define		SR_BSY	0x10
#define		SR_RFF	0x08
#define		SR_RNE	0x04
#define		SR_TNF	0x02
#define		SR_TFE	0x01

#define SSI_CPSR	0x0010
#define SSI_IM		0x0014
#define SSI_IRIS	0x0018
#define SSI_MIS		0x001C
#define SSI_ICR		0x0020
#define SSI_DMACTL	0x0024
#define SSI_PP		0x0FC0
#define SSI_CC		0x0FC8

#define SYSCON_RCGSSI	(0x400FE000 + 0x061C)
#define SYSCON_PCSSI	(0x400FE000 + 0x091C)

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
  { .base = SSI_BASE(0),
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = SSI_BASE(1),
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = SSI_BASE(2),
    .modebits = 0xffffffffU,
    .nbits = 0U,
    .frequency = { .requested = 0U,
                   .actual = 0U },
    .txbuffer = NULL,
    .rxbuffer = NULL,
    .ntxwords = 0U,
    .nrxwords = 0U,
    .nwords = 0U },
  { .base = SSI_BASE(3),
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
  return inl(ssi_ports[port].base + offset);
}

static inline void ssi_putreg(int port, unsigned int offset, uint32_t value)
{
  outl(ssi_ports[port].base + offset, value);
}

static uint32_t ssi_disable(int port)
{
  uint32_t regval;
  uint32_t retval = ssi_getreg(port, SSI_CR1);

  regval = (retval & (~CR1_SSE));
  ssi_putreg(port, SSI_CR1, regval);
  return retval;
}

static void ssi_enable(int port, uint32_t enable)
{
  uint32_t regval = ssi_getreg(port, SSI_CR1);

  regval &= (~CR1_SSE);
  regval |= (enable & CR1_SSE);
  ssi_putreg(port, SSI_CR1, regval);
}

static void ssi_setmodebits_unlocked(int port, uint32_t modebits)
{
  uint32_t regval;

  if (modebits != (ssi_ports[port].modebits)) {
    regval  = ssi_getreg(port, SSI_CR0);
    regval &= (~(CR0_FRF | CR0_SPH | CR0_SPO));
    regval |= modebits;
    ssi_putreg(port, SSI_CR0, regval);
    ssi_ports[port].modebits = modebits;
  }
}

static void ssi_setnbits_unlocked(int port, unsigned int nbits)
{
  uint32_t regval;

  if ((nbits != (ssi_ports[port].nbits)) && (nbits >= 4) && (nbits <= 16)) {
    regval  = ssi_getreg(port, SSI_CR0);
    regval &= (~CR0_DSS);
    regval |= nbits - 1U;
    ssi_putreg(port, SSI_CR0, regval);
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
    if (frequency > (SYS_CLOCK / 2U))
      frequency = (SYS_CLOCK / 2U);
    maxdvsr = (SYS_CLOCK / frequency);
    cpsdvsr = 0U;
    do {
      cpsdvsr += 2U;
      scr = ((maxdvsr / cpsdvsr) - 1U);
    } while (scr > 255U);
    ssi_putreg(port, SSI_CPSR, cpsdvsr);
    regval = ssi_getreg(port, SSI_CR0);
    regval &= ~CR0_SCR;
    regval |= scr << CR0_SCR_SHIFT;
    ssi_putreg(port, SSI_CR0, regval);
    ssi_ports[port].frequency.requested = frequency;
    ssi_ports[port].frequency.actual = (SYS_CLOCK /
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
  return 0U == (ssi_getreg(port, SSI_SR) & SR_RNE);
}

static inline bool ssi_txfifofull(int port)
{
  return 0U == (ssi_getreg(port, SSI_SR) & SR_TNF);
}

void ssi_txnull(int port)
{
  ssi_putreg(port, SSI_DR, 0xffffU);
}

static void ssi_txuint16(int port)
{
  const uint16_t *ptr = ((const uint16_t *)(ssi_ports[port].txbuffer));

  ssi_putreg(port, SSI_DR, (uint32_t)(*ptr++));
  ssi_ports[port].txbuffer = ptr;
}

static void ssi_txuint8(int port)
{
  const uint8_t *ptr = ((const uint8_t *)(ssi_ports[port].txbuffer));

  ssi_putreg(port, SSI_DR, ((uint32_t)(*ptr++)));
  ssi_ports[port].txbuffer = ptr;
}

void ssi_rxnull(int port)
{
  ssi_getreg(port, SSI_DR);
}

static void ssi_rxuint16(int port)
{
  uint16_t *ptr = ((uint16_t *)(ssi_ports[port].rxbuffer));

  *ptr = ((uint16_t)(ssi_getreg(port, SSI_DR)));
  ssi_ports[port].rxbuffer = (++ptr);
}

static void ssi_rxuint8(int port)
{
  uint8_t *ptr = ((uint8_t *)(ssi_ports[port].rxbuffer));

  *ptr = ((uint8_t)(ssi_getreg(port, SSI_DR)));
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

static void ssi_power_up(unsigned int port)
{
  tm4c129x_modreg(SYSCON_PCSSI, 0U, 1U << port);
  tm4c129x_modreg(SYSCON_RCGSSI, 0U, 1U << port);
}

void ssi_init(int which)
{
  switch (which) {
  case 0:
    ssi_power_up(0U);
    break;
  case 1:
    ssi_power_up(1U);
    break;
  case 2:
    ssi_power_up(2U);
    break;
  case 3:
    ssi_power_up(3U);
    /* PQ0/SSI3CLK */
    gpio_setup_pin(GPIO_PORT('Q'), GPIO_PINFUN_PFIO, 0U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
    /* PQ1/SSI3FSS */
    gpio_setup_pin(GPIO_PORT('Q'), GPIO_PINFUN_PFIO, 1U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
    /* PQ2/SSI3XDAT0 */
    gpio_setup_pin(GPIO_PORT('Q'), GPIO_PINFUN_PFIO, 2U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
    /* PF0/SSI3XDAT1 */
    gpio_setup_pin(GPIO_PORT('F'), GPIO_PINFUN_PFIO, 0U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
    /* PF4/SSI3XDAT2 */
    gpio_setup_pin(GPIO_PORT('F'), GPIO_PINFUN_PFIO, 4U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
    /* PF5/SSI3XDAT3 */
    gpio_setup_pin(GPIO_PORT('F'), GPIO_PINFUN_PFIO, 5U,
                   GPIO_PAD_STD, 0U, 14U, 0U);
#ifdef CONFIG_EK
    /* PQ3 */
    gpio_setup_pin(GPIO_PORT('Q'), GPIO_PINFUN_O, 3U,
                   GPIO_PAD_STD, 0U, 0U, 0U);
#else
    /* PH4 */
    gpio_setup_pin(GPIO_PORT('H'), GPIO_PINFUN_O, 4U,
                   GPIO_PAD_STD, 0U, 0U, 0U);
#endif
    gpio_write(GPIO_PORT('Q'), 1U, 1U);
#ifdef CONFIG_EK
    gpio_write(GPIO_PORT('Q'), 3U, 1U);
#else
    gpio_write(GPIO_PORT('H'), 4U, 1U);
#endif
    break;
  default:
    for (;;)
      asm("wfe");
  }
  ssi_putreg(which, SSI_CR1, 0U);
  ssi_putreg(which, SSI_CR0, 0U);
  ssi_setmodebits_unlocked(which, 0U);
  ssi_setnbits_unlocked(which, 8U);
  ssi_setfrequency_unlocked(which, 400000U);
  ssi_putreg(which, SSI_IM, 0U);
  ssi_enable(which, CR1_SSE);
}
