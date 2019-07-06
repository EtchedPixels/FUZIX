; 2015-01-17 William R Sowerbutts

                .module monitor
                .include "kernel.def"
                .globl _platform_monitor
                .globl map_kernel

; -----------------------------------------------------------------------------
.ifne USE_FANCY_MONITOR ; -----------------------------------------------------
                .area _CODE ; actual monitor lives in kernel bank
                .include "../lib/monitor-z80.s"

                .area _COMMONMEM ; just a stub goes in common memory
_platform_monitor:
                di
                call map_kernel
                jp monitor_entry


; -----------------------------------------------------------------------------
.else ; MICRO MONITOR ---------------------------------------------------------
                .globl outchar
                .globl outnewline
                .globl outhl

                .area _COMMONMEM
_platform_monitor:  di
                call outnewline
                ; just dump a few words from the stack
                ld b, #50
stacknext:      pop hl
                call outhl
                ld a, #' '
                call outchar
                djnz stacknext
                halt
.endif
