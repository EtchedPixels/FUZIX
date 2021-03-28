#ifndef __TM4C129X_H
#define __TM4C129X_H

#include <stdint.h>

#define SDCARD_SSI_PORT 3

#if 0
#define ELF32
#define PROGSIZE 65536U
extern uint8_t progbase[PROGSIZE];
#define PROGBASE ((uintptr_t)(&progbase))
#define PROGBASE_HAS_EXPR
#define PROGLOAD PROGBASE
#define PROGTOP (PROGBASE + PROGSIZE)
#endif

void tm4c129x_init(void);

#endif /* __TM4C129X_H */
