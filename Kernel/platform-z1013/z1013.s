;
;	Minimal Z1013 support
;
        .module z1013

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_process_di
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl map_buffers
	.globl plt_interrupt_all
	.globl _plt_reboot
	.globl _plt_monitor
	.globl _bufpool
	.globl _int_disabled

        ; imported symbols
	.globl init
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline

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
	ld hl,#64
	ld (_ramsize), hl
	ld hl,#32
	ld (_procmem), hl

	im 2				; set Z80 CPU interrupt mode 2
	ret

;=========================================================================
; Common Memory (mapped low below ROM)
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
	rst 0x38			; To monitor
_plt_reboot:
	call map_kernel			; ROM in
	jp init

;=========================================================================

_int_disabled:
	.db 1

; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
	ret

;=========================================================================
; Memory management
;=========================================================================

;
rom_state:
	.byte 1		;	ROM starts mapped
save_rom:
	.byte 0		;	For map_save/restore

;
;	Centralize all control of the toggle in one place so we can debug it
;
rom_in:
	in a,(4)
	set 5,a
	out (4),a
	ld (rom_state),a
	ret

rom_out:
	in a,(4)
	res 5,a
	out (4),a
	ld (rom_state),a
	ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
map_process_di:
	ld a,h
	or l				; HL == 0?
	jr z,map_kernel			; HL == 0 - map the kernel

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
; map_process_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_process_always:
map_process_always_di:
	push af
was_u:
	call rom_out
	pop af
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push af
was_k:
	call rom_in
	pop af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push af
	ld a,(save_rom)
	or a
	jr z, was_k
	jr was_u

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push af
	ld a,(rom_state)
	ld (save_rom),a
	call rom_in
	pop af
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
out_done:
	; No useful non video device. FIXME: v24 later
	out (0xFF),a	; emulator tracing
	ret

	.area _COMMONMEM
;
;	Character pending or 0
;
	.globl _ramdp

_ramdp:
	.word	0

	.globl _keycheck
;
_keycheck:
	rst 0x20
	.byte 4
	ld l,a
	ret

ramconf5:
	sla	l		; Turn HL into 256 byte blocks
	rl	h
	ld	a,l
	out	(0x5e),a	; Set the middle byte
	xor	a
	out	(0x5f),a	; Clear the counter
	ld	a,h
	add	#0x58		; Work out which port to use
	ld	b,#0		; Set up a and b for the caller
	ld	a,l
	inc	a
	ld	hl,(_ramdp)	; Set up HL for the caller
	ret

	.globl	_ramread5

_ramread5:
	call	ramconf5
	inir
	out	(0x5e),a
	inir
	ret

	.globl	_ramwrite5

_ramwrite5:
	call	ramconf5
	otir
	out	(0x5e),a
	otir
	ret

	.area _DISCARD

	.globl	_ramdet5

_ramdet5:
	xor	a		;	Point to start of ram disc
	out	(0x5e),a
	out	(0x5f),a
	ld	c,#0x58		;	Read first two bytes
	in	e,(c)
	in	d,(c)
	xor	a
	out	(0x5f),a	;	Reset pointer
	ld	a,#0xAA
	out	(0x58),a	;	Write AA55
	cpl
	out	(0x58),a
	xor	a
	out	(0x5f),a	;	Reset pointer
	in	a,(0x58)
	cp 	#0xAA		;	Should get AA55 back
	jr	nz, nodisk
	in	a,(0x58)
	cp	#0x55
	jr	nz, nodisk
	; Success - put back the old bytes
	xor	a
	out	(0x5f),a
	out	(c),e
	out	(c),d
	ld	l,#1
	ret
nodisk:
	ld	l,#0
	ret
