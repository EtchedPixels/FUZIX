#include <kernel.h>

void vfd_term_init(void) {
  __asm
       call VFDTERM_PREINIT
  __endasm;
}

void vfd_term_write(char c) {
  __asm
        ld      hl, #4+0		; banked
        add     hl, sp
        ld      e, (hl)
        call    VFDTERM_PUTC
  __endasm;
  (void)c; // suppress error
}

