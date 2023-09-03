;
;    TC2068 hardware support
;

        .module tc2068

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler

        .globl map_kernel
        .globl map_proc_always
        .globl map_proc
        .globl map_kernel_di
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_kernel_restore
	.globl map_for_swap
	.globl map_video
	.globl current_map

        .globl _need_resched
	.globl _int_disabled
	.globl _vtborder

        ; exported debugging tools
        .globl _plt_monitor
	.globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

	.globl _vtoutput
	.globl _vtinit

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (below 0x4000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _plt_monitor

_plt_reboot:
	di
        rst 0		; back into our booter

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0


; -----------------------------------------------------------------------------
; KERNEL CODE BANK (above 0x4000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
	ld a,#0xFF
	ld (current_map),a
	out (0xF4),a		; Map kernel fully
        ret

	.area _VIDEO

init_hardware:
        ; set system RAM size
        ld hl, #80
        ld (_ramsize), hl
        ld hl, #33	      ; 32K for kernel/screen/etc
        ld (_procmem), hl

	ld a,#0x3E
	out (0xFF),a
        ; screen initialization
	call _vtinit

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}
map_proc:
        ld a, h
        or l
        jr z, map_kernel
map_for_swap:
map_proc_always:
map_proc_always_di:
	push af
	ld a,#0x03			; catridge in low 16K only
	ld (current_map),a
	out (0xF4),a
	pop af
	ret
;
;	Save and switch to kernel
;
map_save_kernel:
	push af
        ld a, (current_map)
        ld (map_store), a
	pop af
map_kernel_di:
map_kernel:
map_kernel_restore:
	push af
	ld a,#0xFF			; entirely from cartridge
	ld (current_map),a
	out (0xF4),a
	pop af
	ret

map_video:
	push af
	ld a,#0xF3			; entirely from cartridge except video
	ld (current_map),a
	out (0xF4),a
	pop af
	ret

map_restore:
	push af
        ld a, (map_store)
	ld (current_map),a
	out (0xF4),a
	pop af
	ret

;
;	We have no easy serial debug output instead just breakpoint this
;	address when debugging.
;
outchar:
	ld (_tmpout), a
	push bc
	push de
	push hl
	push ix
	ld hl, #1
	push hl
	ld hl, #_tmpout
	push hl
	call _vtoutput
	pop af
	pop af
	pop ix
	pop hl
	pop de
	pop bc
        ret

	.area _COMMONDATA
_tmpout:
	.db 1

current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 0xF4 port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

_need_resched:
        .db 0

