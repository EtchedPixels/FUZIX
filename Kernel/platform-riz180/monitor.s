; 2015-01-17 William R Sowerbutts

                .module monitor
                .include "kernel.def"
	        .include "../cpu-z180/z180.def"
                .globl _plt_monitor
		.globl _plt_reboot
                .globl map_kernel

                .globl outchar
                .globl outnewline
                .globl outhl

                .area _COMMONMEM

; -----------------------------------------------------------------------------
.ifne USE_FANCY_MONITOR ; -----------------------------------------------------
                .area _COMMONMEM ; actual monitor lives in high memory
                .include "../lib/monitor-z80.s"

;                .area _COMMONMEM ; just a stub goes in common memory
_plt_monitor:
                di
                jp monitor_entry


; -----------------------------------------------------------------------------
.else ; MICRO MONITOR ---------------------------------------------------------
                 .globl outchar
                 .globl outnewline
                 .globl outhl
@@ -35,7 +22,10 @@ stacknext:      pop hl
                 call outchar
                 djnz stacknext
                 halt
.endif

_plt_reboot:
		; Map ROM for all of low area
		ld a,#0xF0
		out (MMU_CBAR),a
		; Jump into it
		rst 0
