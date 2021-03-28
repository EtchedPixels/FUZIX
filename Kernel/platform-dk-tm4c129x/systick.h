#ifndef __SYSTICK_H
#define __SYSTICK_H

#include <arch/board/board.h>

#include "config.h"

#define SYSTICK_RELOAD ((SYSCLK_FREQUENCY / TICKSPERSEC) - 1U)

void systick_init(void);

#endif /* __SYSTICK_H */
