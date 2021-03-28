#ifndef __EXCEPTIONS_H
#define __EXCEPTIONS_H

#include <stdint.h>

#include <nvic.h>
#include <arch/tiva/tm4c_irq.h>

#define NVIC_ENA_OFFSET    0U
#define NVIC_CLRENA_OFFSET (NVIC_IRQ0_31_CLEAR - NVIC_IRQ0_31_ENABLE)

typedef uint32_t *(*exception_handler)(uint32_t, uint32_t *);

extern exception_handler exception_handlers[NR_IRQS];

void exception_common(void);

void exception_attach_handler(uint32_t exception, exception_handler handler);
void exception_enable(uint32_t exception);
void exception_disable(uint32_t exception);

#endif /* __EXCEPTIONS_H */
