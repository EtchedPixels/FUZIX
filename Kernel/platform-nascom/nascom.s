;
;	    Nascom  hardware support
;

            .module nascom

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl interrupt_handler
        .globl _program_vectors
	.globl plt_interrupt_all
	.globl _int_disabled

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
	.globl map_kernel
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save_kernel
	.globl map_restore

	.globl s__COMMONMEM
	.globl l__COMMONMEM

	.globl _bufpool

        .include "kernel.def"
        .include "../kernel-z80.def"

	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

plt_interrupt_all:
	ret

_plt_monitor:
	rst 0x20

_plt_reboot:
	rst 0x00

nascom_nmi:
	push	af
	ld	a,(_int_disabled)
	or	a
	jr	nz, nmi_event
	pop	af
	retn
	; We took an NMI but interrupts are enabled - fake an interrupt
	; TODO - need to interlock this properly with the IRQ handlers
	; proper if we add an IM2 PIO hook or similar
	;
	; This logic allows for the NMI supporting RTC cards to be used, and
	; also means that the nmi button will break out of stuck user space.
nmi_event:
	pop	af
	jp	interrupt_handler

	.area _COMMONDATA

_int_disabled:
	.byte	1

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
	.area _CODE

init_early:
	im	1
        ret

_program_vectors:
	; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	di ; just to be sure
	pop	de ; temporarily store return address
	pop	hl ; function argument -- base page number
	push	hl ; put stack back as it was
	push	de

	call	map_process

	; write zeroes across all vectors
	ld	hl, #0
	ld	de, #1
	ld	bc, #0x007f ; program first 0x80 bytes only
	ld	(hl), #0x00
	ldir

	; now install the interrupt vector at 0x0038
	ld	a, #0xC3 ; JP instruction
	ld	(0x0038), a
	ld	hl, #interrupt_handler
	ld	(0x0039), hl

	ld	(0x0000), a   
	ld	hl, #null_handler   ;   to Our Trap Handler
	ld	(0x0001), hl

	; Some Nascoms got fitted with an NMI button - what should it do
	; ?
	ld	(0x0066), a  ; Set vector for NMI
	ld	hl, #nmi_key_handler
	ld	(0x0067), hl
	jp	map_kernel

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

	.area _COMMONMEM
;
;	This isn't absolutely perfect but as close as we can get
;
nmi_key_handler:
	push	af
	ld	a,#1
	ld	(_nmikey),a
	pop	af
	; FIXME: take nascom port idea of forcing interrupt and do it here
	; too, also retrofit nmi_key_handler check back into tty check
	retn

outchar:
	push	af
outwait:
	in	a, (0x02)
	bit	6,a
	jr	z, outwait
        out	(0x1), a
	ret

	.area	_COMMONDATA
_nmikey:
	.byte	0
