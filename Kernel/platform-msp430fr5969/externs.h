#ifndef EXTERNS_H
#define EXTERNS_H

#include "globals.h"

extern void sd_rawinit(void);
extern void tty_rawinit(void);
extern void tty_interrupt(void);
extern void fuzix_main(void);
extern void load_overlay(uint16_t start, uint16_t stop);
extern void load_overlay_for_syscall(void);

#endif

