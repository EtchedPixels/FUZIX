; 2015-01-17 William R Sowerbutts

#include "kernelu.def"

                .export _plt_monitor

#ifdef USE_FANCY_MONITOR
                .code ; actual monitor lives in kernel bank
#include "../../lib/monitor-z80u.S"

                .common ; just a stub goes in common memory
_plt_monitor:
                di
                call map_kernel
                jp monitor_entry


#else
		.common

_plt_monitor:  di
                call outnewline
                ; just dump a few words from the stack
                ld b, 50
stacknext:      pop hl
                call outhl
                ld a, ' '
                call outchar
                djnz stacknext
                halt
#endif
