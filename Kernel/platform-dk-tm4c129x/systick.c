#include <kernel.h>
#include "tm4c129x.h"
#include "interrupt.h"
#include "systick.h"

static irqreturn_t systick_isr(unsigned int irq, void *dev_id, uint32_t *regs)
{
  timer_interrupt();
  return IRQ_HANDLED;
}

void systick_init(void)
{
  uint32_t regval;

  regval = inl(NVIC_SYSH12_15_PRIORITY);
  regval &= (~(NVIC_SYSH_PRIORITY_PR15_MASK));
  regval |= (NVIC_SYSH_PRIORITY_DEFAULT << NVIC_SYSH_PRIORITY_PR15_SHIFT);
  outl(NVIC_SYSH12_15_PRIORITY, regval);
  outl(NVIC_SYSTICK_RELOAD, SYSTICK_RELOAD); /* requested rate */
  outl(NVIC_SYSTICK_CTRL, (NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT |
            NVIC_SYSTICK_CTRL_ENABLE));
  request_irq(IRQ_SYSTICK, systick_isr, NULL);
}
