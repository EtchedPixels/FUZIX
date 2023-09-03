;
;	Basic VZ200 setup
;
        .module vz200

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_proc_di
	.globl map_proc_save
	.globl map_proc_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl map_buffers
	.globl plt_interrupt_all
	.globl _plt_reboot
	.globl _plt_monitor
	.globl _bufpool
	.globl _int_disabled
	.globl _interrupt_setup

        ; imported symbols
	.globl init
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl nmi_handler
	.globl null_handler

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../kernel-z80.def"

;=========================================================================
; Buffers
;=========================================================================
        .area _BUFFERS
	.globl kernel_endmark

_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this
;
;	So we can check for overflow
;
kernel_endmark:

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
	ld	hl,#90
	ld	(_ramsize), hl
	ld	hl,#30
	ld	(_procmem), hl
	ret

;=========================================================================
; Common Memory between 7800-87FF
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	di
	halt

; Install the interrupt vector now the discard can be trashed
_interrupt_setup:
	ld	hl,#0x787D
	ld	(hl),#0xC3
	ld	de,#interrupt_stub
	inc	 hl
	ld	(hl),e
	inc	hl
	ld	(hl),d
	ei
	xor	a
	ld	(_int_disabled),a
	inc	a
	ld	(_postinit),a
	ret

interrupt_stub:
	pop	hl		; return
	pop	hl
	pop	de
	pop	bc
	pop	af		; unwind rom
	jp	interrupt_handler	; and do our thing

;=========================================================================

		.globl _postinit

_int_disabled:
	.db 1
_postinit:
	.db 0

; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
	ret

;
;	Custom interrupt handling. We have to keep interrupts off until
;	after early boot.
;
		.globl _di
		.globl ___hard_di

_di:
	jp	___hard_di

		.globl _ei
		.globl ___hard_ei

_ei:
	ld	a,(_postinit)
	or	a
	ret	z
	jp	___hard_ei

		.globl _irqrestore

_irqrestore:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	di
	ld	a,l
	ld	(_int_disabled),a
	or	a
	ret	nz
	ld	a,(_postinit)
	or	a
	ret	z
	ei
	ret

;=========================================================================
; Memory management
;=========================================================================

;
;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
	ld	a,h
	or	l			; HL == 0?
	jr	z,map_kernel		; HL == 0 - map the kernel

	; fall through

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
;=========================================================================
map_for_swap:

	; fall through

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_save:		; FIXME: fix banked 
map_proc_always:
map_proc_always_di:
	push af
	ld	a,(mapreg)
	ld	(oldmap),a	; remember our bank
	ld	a,#0x7		; RAM low, bank 1
	ld	(mapreg),a
	out	(55),a
	pop	af
	ret

;
;	On a banked kernel map_buffers will map the disk buffers and a
;	map_kernel_restore will follow. We don't need to do any work but
;	we do need to make the map_kernel_restore behave properly.
;
map_buffers:
	ld	a,(mapreg)
	ld	(oldmap),a
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
	ld	a,(oldmap)
	ld	(mapreg),a
	out	(55),a
	pop	af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push	af
	ld	a,(save_map)
	ld	(mapreg),a
	out	(55),a
	pop	af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push	af
	ld	a,(mapreg)
	ld	(save_map),a
	ld	a,#3
	ld	(mapreg),a
	out	(55),a
	pop	af
	ret

save_map:
	.byte	0
oldmap:
	.byte	0x01
mapreg:
	.byte	0x01		; start with a map setting of 1

;
;	Helpers specific to the banked singe task support
;
	.globl map_save_kmap
	.globl map_restore_kmap

map_save_kmap:
	ld	a,(mapreg)
	ret
map_restore_kmap:
	ld	(mapreg),a
	out	(55),a
	ret


;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	ret

;
	.area _COMMONMEM

;	Banking support
;
	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_1_2
	.globl __bank_2_1
	.globl __stub_0_1
	.globl __stub_0_2

callhl:
	jp	(hl)
__bank_0_1:
	ld	a,#1
	jr	bank0
__bank_0_2:
	ld	a,#3
bank0:
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	ex	de,hl
	ld	bc,(mapreg)	; old bank into C
	ld	(mapreg),a
	out	(55),a		; Switch bank
	bit	1,c		; Which bank ?
	jr	z, retbank1
retbank2:
	call	callhl
	ld	a,#3
	ld	(mapreg),a
	out	(55),a		; and back
	ret
retbank1:
	call	callhl
	ld	a,#1
	ld	(mapreg),a
	out	(55),a
	ret
__bank_1_2:
	ld	a,#3
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	ld	(mapreg),a
	out	(55),a
	ex	de,hl
	call	callhl
	ld	a,#1
	ld	(mapreg),a
	out	(55),a
	ret
__bank_2_1:
	ld	a,#1
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	ld	(mapreg),a
	out	(55),a
	ex	de,hl
	call	callhl
	ld	a,#3
	ld	(mapreg),a
	out	(55),a
	ret
__stub_0_1:
	ld	a,#1
	jr	stubin
__stub_0_2:
	ld	a,#3
stubin:
	pop	hl
	ex	(sp),hl
	ex	de,hl
	ld	bc,(mapreg)
	ld	(mapreg),a
	out	(55),a
	bit	1,c
	jr	z, from_1
	call	callhl
	ld	a,#3
stubout:
	ld	(mapreg),a
	out	(55),a
	pop	bc
	push	bc
	push	bc
	ret
from_1:
	call	callhl
	ld	a,#1
	jr	stubout
