;
;    ZX Uno hardware support
;

        .module zxuno

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler

        .globl map_kernel
        .globl map_process_always
        .globl map_kernel_di
        .globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_process_save
	.globl map_kernel_restore
	.globl map_for_swap
	.globl map_process_a
	.globl map_video_save

        .globl _need_resched
	.globl _int_disabled
	.globl _portff

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
	im 1
	ld bc, #0x7FFD
	xor a		; 128K ROM, initial banks, low screen
	; FIXME: will need to put the out(c),a at 0xFFFE and
	; jump to it
	out (c), a
        rst 0		; Into the ROM

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1


; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0x4000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE1

;
;	The memory banker will deal with the map setting
;
init_early:
        ret

	.area _COMMONMEM

init_hardware:
        ; set system RAM size
	; sort of anyway - we need to revisit this with the real values
	; once we have the rest of the map worked out
        ld	hl, #192
        ld	(_ramsize), hl
        ld	hl, #96		; 64K for kernel 16K for screen and lose some
				; to common
        ld	(_procmem), hl

        ; screen initialization
	ld	a,(_portff)
	out	(0xff),a

	jp	_vtinit

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	ret

map_process_a:
	push	af
	push	bc
	ld	c,a
	ld	a,#0xFC
	out	(0xF4),a
	ld	a,c
	jr	map_res_a

map_proc_hl:
	push	af
	push	bc
	ld	a,#0xFC		; All but low 16K from dock or ex
	out	(0xF4),a
	ld	a,(hl)
map_res_a:
	ld	(user_map),a
	dec	a
	ld	a,(_portff)
	ld	c,#0x80		; 2 is EXROM
	jr	nz, is_exr
	ld	c,#0		; 1 is DOCK
is_exr:
	and	#0x7F		; Clear the dock/exrom bit
	or	c		; Merge in the new one
	ld	(_portff),a	; Switch
	out	(0xFF),a
	pop	bc
	pop	af
	ret

map_process_save:
map_process_always:
map_process_always_di:
map_for_swap:
	push	hl
	ld	hl,#U_DATA__U_PAGE
	call	map_proc_hl
	pop	hl
	ret
;
;	Save and switch to kernel
;
map_save_kernel:
	push	af
	push	bc
	ld	a,(user_map)
	ld	(user_store),a
        ld	a, (high_map)
        ld	(high_store), a
	ld	a,#0x0B		; bank 3 screen 7
	ld	(high_map),a
	ld	bc,#0x7FFD
	out	(c),a
	xor	a
	ld	(user_map),a
	out	(0xF4),a
	pop	bc
	pop	af
	ret
;
;	Kernel - clear the bank register so we get standard memory
;	(plus the DIVMMC space)
;
map_kernel_di:
map_kernel:
map_kernel_restore:
	push	af
	push	bc
	xor	a
	ld	(user_map),a
	out	(0xF4),a
	ld	a,#0x0B		; bank 3 screen 7
	ld	bc,#0x7FFD
	ld	(high_map),a
	out	(c),a
	pop	bc
	pop	af
	ret
;
;	Temporarily switch to video memory, this lives in bank 7 so replaces
;	some of the kernel. This is fine as we are in common
;
map_video_save:
	push	af
	push	bc
	ld	a,#0x0F		; bank 7 screen 7
map_set_high:
	ld	(high_map),a
	ld	bc,#0x7FFD
	out	(c),a
	pop	bc
	pop	af
	ret

map_restore:
	push	af
	push	bc
	ld	a, (user_store)
	ld	(user_map),a
	or	a
	jr	nz, map_restore_u
	out	(0xF4),a
	ld	a,(high_store)
	jr	map_set_high
map_restore_u:
	ld	a,#0xFC
	out	(0xF4),a
	ld	a,(user_map)
	jp	map_res_a


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

user_map:
	.db	0
user_store:
	.db	0
high_map:
	.db	0
high_store:
	.db	0

_portff:
	.db	0x3E	    ; High resolution white on black
