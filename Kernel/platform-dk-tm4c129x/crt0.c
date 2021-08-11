#include <kernel.h>
#include "inline-irq.h"
#include "tm4c129x.h"
#include "entries.h"

extern uint8_t _sbss;
extern uint8_t _ebss;
extern uint8_t _eronly;
extern uint8_t _sdata;
extern uint8_t _edata;

_Noreturn void start(void)
{
  const uint8_t *src;
  uint8_t *dest;
  uint8_t *up = &_sbss;

  __hard_di();
  while (up != (&_ebss))
    *(up++) = 0U;
  for (src = &_eronly, dest = &_sdata; dest < &_edata;)
    *dest++ = *src++;
  tm4c129x_init();
  fuzix_main();
  for (;;)
    asm("wfe");
}
