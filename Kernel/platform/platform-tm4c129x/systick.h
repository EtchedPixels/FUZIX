#ifndef __SYSTICK_H
#define __SYSTICK_H

#define SYSTICK_RELOAD ((SYS_CLOCK / TICKSPERSEC) - 1U)

void systick_init(void);

#endif /* __SYSTICK_H */
