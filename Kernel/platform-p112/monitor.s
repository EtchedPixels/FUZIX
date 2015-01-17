; 2015-01-17 William R Sowerbutts
                .module monitor
                .include "kernel.def"

.ifne USE_FANCY_MONITOR
                .include "../lib/monitor-z80.s"
.else
                .globl outchar
                .globl outnewline
                .globl outhl
                .globl _trap_monitor

                ; micro monitor -
                ; just dumps a few words from the stack
_trap_monitor:  di
                call outnewline
                ld b, #50
stacknext:      pop hl
                call outhl
                ld a, #' '
                call outchar
                djnz stacknext
                halt
.endif
