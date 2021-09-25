#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#define NR_IRQS		(130)	/* Sort out exact value TODO */
#define SYS_NUM_IRQ	16

#define IRQ_MEMFAULT	3
#define IRQ_BUSFAULT	4
#define IRQ_USGFAULT	5
#define IRQ_SVC		11
#define IRQ_DEBUG	12
#define IRQ_SYSTICK	15

#define IRQ_HANDLED	1
#define IRQ_NONE	0

/* NVIC sources */

#define IRQ_UART0	21
#define IRQ_UART1	22
#define IRQ_UART2	49
#define IRQ_ETHCON	56

typedef unsigned int irqreturn_t;

typedef irqreturn_t (*irq_handler_t)(unsigned int, void *, uint32_t *);

extern irq_handler_t exception_handlers[NR_IRQS];

int request_irq(unsigned int irq, irq_handler_t handler, void *dev_id);
void free_irq(unsigned int irq, void *dev_id);
void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);

void exception_common(void);

#endif /* __INTERRUPT_H */
