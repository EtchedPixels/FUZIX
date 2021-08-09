/*
 *	We don't yet really have a general structure for interruts on
 *	FUZIX in complex setups but the Cromemco which does apes the Linux
 *	API so we shall do the same in case we want to move it into generic
 *	code
 */

#include <kernel.h>
#include "tm4c129x.h"
#include "interrupt.h"

/* A call to an unused interrupt line */
static irqreturn_t null_handler(unsigned int irq, void *dev_id, uint32_t *regs)
{
 return IRQ_HANDLED;
}

static irq_handler_t irq_handler[NR_IRQS] = {
  [0 ... (NR_IRQS - 1)] = null_handler
};

static void *irq_dev[NR_IRQS] = {
  [0 ... (NR_IRQS - 1)] = NULL
};

static uint32_t sysirq_bit[SYS_NUM_IRQ] = {
  0U,
  0U,
  0U,
  0U,
  NVIC_SYSHCON_MEMFAULTEN,
  NVIC_SYSHCON_BUSFAULTEN,
  NVIC_SYSHCON_USGFAULTEN,
  0U,
  0U,
  0U,
  0U,
  0U,
  0U,
  0U,
  0U,
  NVIC_SYSTICK_CTRL_ENABLE
};

static uint32_t sysirq_port[SYS_NUM_IRQ] = {
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSHCON,
  NVIC_SYSTICK_CTRL
};

/* We have two different interrupt controllers, the NVIC and the SYSCON. The
   NVIC has 128 interrupts, the SYSCON just a few and mostly they are really
   error handlers */

void enable_irq(unsigned int irq)
{
    if (irq >= SYS_NUM_IRQ) {
        irq -= SYS_NUM_IRQ;
        /* Enable the NVIC register bit */
        outl(NVIC_ENABLE + ((irq >> 5) << 2), 1 << (irq & 31));
    } else {
        /* SYSCON has a register per irq */
        outmod32(sysirq_port[irq], sysirq_bit[irq], sysirq_bit[irq]);
    }
}

void disable_irq(unsigned int irq)
{
    if (irq >= SYS_NUM_IRQ) {
        irq -= SYS_NUM_IRQ;
        /* Disable the NVIC register bit */
        outl(NVIC_CLRENABLE + ((irq >> 5) << 2), 1 << (irq & 31));
    } else {
        /* SYSCON has a register per irq */
        outmod32(sysirq_port[irq],  sysirq_bit[irq], 0);
    }
}

/* Minimal for now - one handler per IRQ */

int request_irq(unsigned int irq, irq_handler_t handler, void *dev_id)
{
    if (irq >= NR_IRQS)
        return -EINVAL;
    if (irq_dev[irq])
        return -EBUSY;
    irq_dev[irq] = dev_id;
    irq_handler[irq] = handler;
    enable_irq(irq);
    return 0;
}

void free_irq(unsigned int irq, void *dev_id)
{
    disable_irq(irq);
    irq_dev[irq] = NULL;
    irq_handler[irq] = null_handler;
}

/* Glue */
uint32_t *handle_exception(uint32_t irq, uint32_t *regs)
{
    irq_handler[irq](irq, irq_dev[irq], regs);
    return regs;
}
