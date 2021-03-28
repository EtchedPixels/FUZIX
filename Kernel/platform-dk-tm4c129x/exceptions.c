#include <stddef.h>
#include <stdint.h>

#include <nvic.h>
#include <arch/tiva/tm4c_irq.h>
#include <arch/tiva/irq.h>

#include "cpu.h"

#include "exceptions.h"

exception_handler exception_handlers[NR_IRQS] = {
  [0 ... (NR_IRQS - 1)] = NULL
};

uint32_t *handle_exception(uint32_t exception, uint32_t *regs)
{
  return exception_handlers[exception] ?
          exception_handlers[exception](exception, regs) : regs;
}

void exception_attach_handler(uint32_t exception, exception_handler handler)
{
  exception_handlers[exception] = handler;
}

static int exception_info(uint32_t exception, uintptr_t *regaddr, uint32_t *bit,
                          uintptr_t offset)
{
  if (exception >= TIVA_IRQ_INTERRUPTS) {
    if (exception >= NR_IRQS)
      return -1;
    if (exception < (TIVA_IRQ_INTERRUPTS + 32U)) {
      *regaddr = (NVIC_IRQ0_31_ENABLE + offset);
      *bit     = 1U << (exception - TIVA_IRQ_INTERRUPTS);
    } else if (exception < (TIVA_IRQ_INTERRUPTS + 64U)) {
      *regaddr = (NVIC_IRQ32_63_ENABLE + offset);
      *bit     = 1U << (exception - TIVA_IRQ_INTERRUPTS - 32U);
    } else if (exception < (TIVA_IRQ_INTERRUPTS + 96U)) {
      *regaddr = (NVIC_IRQ64_95_ENABLE + offset);
      *bit     = 1U << (exception - TIVA_IRQ_INTERRUPTS - 64U);
    } else if (exception < (TIVA_IRQ_INTERRUPTS + 128)) {
      *regaddr = (NVIC_IRQ96_127_ENABLE + offset);
      *bit     = 1U << (exception - TIVA_IRQ_INTERRUPTS - 96U);
    } else
      return -1;
  } else {
    *regaddr = NVIC_SYSHCON;
    if (exception == TIVA_IRQ_MEMFAULT) {
      *bit = NVIC_SYSHCON_MEMFAULTENA;
    } else if (exception == TIVA_IRQ_BUSFAULT) {
      *bit = NVIC_SYSHCON_BUSFAULTENA;
    } else if (exception == TIVA_IRQ_USAGEFAULT) {
      *bit = NVIC_SYSHCON_USGFAULTENA;
    } else if (exception == TIVA_IRQ_SYSTICK) {
      *regaddr = NVIC_SYSTICK_CTRL;
      *bit = NVIC_SYSTICK_CTRL_ENABLE;
    } else
      return -1;
  }
  return 0;
}

void exception_enable(uint32_t exception)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (!(exception_info(exception, &regaddr, &bit, NVIC_ENA_OFFSET))) {
    if (exception >= TIVA_IRQ_INTERRUPTS) {
      putreg32(bit, regaddr);
    } else {
      regval = getreg32(regaddr);
      regval |= bit;
      putreg32(regval, regaddr);
    }
  }
}

void exception_disable(uint32_t exception)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (!(exception_info(exception, &regaddr, &bit, NVIC_CLRENA_OFFSET))) {
    if (exception >= TIVA_IRQ_INTERRUPTS) {
      putreg32(bit, regaddr);
    } else {
      regval = getreg32(regaddr);
      regval &= ~bit;
      putreg32(regval, regaddr);
    }
  }
}
