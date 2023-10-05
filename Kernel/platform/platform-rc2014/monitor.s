; 2015-01-17 William R Sowerbutts

                .module monitor
                .include "kernel.def"
                .globl _plt_monitor
                .globl map_kernel

; -----------------------------------------------------------------------------
.ifne USE_FANCY_MONITOR ; -----------------------------------------------------
                .area _CODE3 ; actual monitor lives in kernel bank
                .include "../../lib/monitor-z80.s"

                .area _COMMONMEM ; just a stub goes in common memory
_plt_monitor:
                di
		ld a,#0x26
		out (0x79),a
		inc a
		out (0x7A),a
                jp monitor_entry

; -----------------------------------------------------------------------------
.else ; MICRO MONITOR ---------------------------------------------------------
                .globl outchar
                .globl outnewline
                .globl outhl

                .area _COMMONMEM
_plt_monitor:  di
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
