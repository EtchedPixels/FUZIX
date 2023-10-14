;
;	    Genie IIs  hardware support
;
        .module genieiis

        ; exported symbols
        .globl interrupt_handler
        .globl _program_vectors
	.globl plt_interrupt_all
	.globl map_kernel
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save_kernel
	.globl map_restore

	.globl _sysport

	.globl go_fast
	.globl go_slow

	.globl _mmio_read
	.globl _mmio_write

	.globl s__COMMONMEM
	.globl l__COMMONMEM

	.globl _int_disabled

	; hard disk helpers
	.globl _hd_xfer_in
	.globl _hd_xfer_out
	; and the page from the C code
	.globl _hd_page

        ; exported debugging tools
        .globl _plt_monitor
        .globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl istack_top
        .globl istack_switched_sp
        .globl unix_syscall_entry
        .globl outcharhex
	.globl null_handler
	.globl _vtflush

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xC000 upwards after the udata etc)
; -----------------------------------------------------------------------------
	    .area _COMMONMEM

_plt_monitor:
	    push af
	    call _vtflush		; get any panic onscreen
	    pop af
monitor_spin:
	    di
	    jr monitor_spin

plt_interrupt_all:
	    ret

_plt_reboot:
	    di
	    ld sp,#0xffff
	    xor a
	    out (0xFE),a
	    rst 0

_int_disabled:
	    .db 1

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

	.area _COMMONMEM
;
;	Genie IIs speed control. Save anything used except AF
;	Only allowed to mess with AF
;
go_slow:
	ld	hl,#_sysport
	set	6,(hl)		; keeps port correct with respect to IRQ
	ld	a,(hl)
	out	(0xFE),a
	ret
go_fast:
	ld	hl,#_sysport
	res	6,(hl)
	out	(0xFE),a

_program_vectors:
	; FIXME: TODO - needed for this port - and call from init_hardware
	ret

_rom_vectors:
	; TODO
	ld a,#0xC3
	ld (0x4012), a
            ld hl, #interrupt_handler
            ld (0x4013), hl

            ; set restart vector for UZI system calls
            ld (0x400F), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x4010), hl

	    ; Model III only but just writing it does no harm
;	    ld hl,#fd_nmi_handler
	    ld (0x404A), hl
	    jp map_kernel
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	; TODO SIO
	ret

;
;	Swap helpers.
;	FIXME: correct port - no longer C8
;
_hd_xfer_in:
	pop	de
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	de
	ld	a, (_hd_page)
	or	a
	push	af
	call	nz, map_process_a
	ld	bc, #0xC8			; 256 bytes from 0xC8
	inir
	pop	af
	ret	z
	jp	map_kernel

_hd_xfer_out:
	pop	de
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	de
	ld	a, (_hd_page)
	or	a
	push	af
	call	nz, map_process_a
	ld	bc, #0xC8			; 256 bytes to 0xC8
	otir
	pop	af
	ret	z
	jp	map_kernel

;
;	I/O access helpers
;

_mmio_read:
	di
	ld	a,(_sysport)
	or	#1
	out	(0xFE),a
	ld	l,(hl)
mmio_out:
	dec	a
	out	(0xFE),a
	ld	a,(_int_disabled)
	or	a
	ret	nz
	ei
	ret

_mmio_write:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	ld	a,(_sysport)
	or 	#1
	out	(0xFE),a
	ld	(hl),e
	jr	mmio_out
