#include <stdint.h>

#include <nvic.h>
#include <arch/tiva/chip.h>
#include <arch/tiva/irq.h>

#include <kernel.h>

#include "cpu.h"

#include "exceptions.h"

#include "systick.h"

static uint32_t *systick_isr(uint32_t irq, uint32_t *regs)
{
  timer_interrupt();
  return regs;
}

void systick_init(void)
{
  uint32_t regval;

  regval = getreg32(NVIC_SYSH12_15_PRIORITY);
  regval &= (~(NVIC_SYSH_PRIORITY_PR15_MASK));
  regval |= (NVIC_SYSH_PRIORITY_DEFAULT << NVIC_SYSH_PRIORITY_PR15_SHIFT);
  putreg32(regval, NVIC_SYSH12_15_PRIORITY);
  putreg32(SYSTICK_RELOAD, NVIC_SYSTICK_RELOAD); /* requested rate */
  exception_attach_handler(TIVA_IRQ_SYSTICK, systick_isr);
  putreg32((NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT |
            NVIC_SYSTICK_CTRL_ENABLE), NVIC_SYSTICK_CTRL);
  exception_enable(TIVA_IRQ_SYSTICK);
}
