#ifndef __INLINE_IRQ_H
#define __INLINE_IRQ_H

#include "types.h"

inline static irqflags_t __hard_di(void)
{
  irqflags_t ps;

  asm volatile (
    "\tmrs    %0, primask\n"
    "\tcpsid  i\n"
     : "=r" (ps)
     :
     : "memory");
  return ps;
}

inline static void __hard_ei(void)
{
  asm volatile("\tcpsie i\n");
}

inline static void __hard_irqrestore(irqflags_t ps)
{
  asm volatile (
    "\ttst    %0, #1\n"
    "\tbne.n  1f\n"
    "\tcpsie  i\n"
    "1:\n"
      :
      : "r" (ps)
      : "memory");
}

#endif /* __INLINE_IRQ_H */
