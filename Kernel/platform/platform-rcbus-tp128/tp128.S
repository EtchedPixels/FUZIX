;
;	Tadeus 128K RAM support. Thiis has a slightly odd mapping scheme
;	where the upper 32K is page between 3 banks but the lower bank is
;	either fixed or ROM (with RAM write through)
;
;	MMU setting via 0xC0 bits 1:0
;	00	Low ROM (writes bank 0)		High bank 1
;	01	Low bank 0			High bank 1
;	10	Low bank 0			High bank 2
;	11	Low bank 0			High bank 3
;
;	This first chunk is mostly boilerplate to adjust for each
;	system.
;

	.export init_early
	.export init_hardware
	.export _program_vectors
	.export map_buffers
	.export map_kernel
	.export map_kernel_di
	.export map_kernel_restore
	.export map_proc
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap
	.export plt_interrupt_all
	.export _kernel_flag
	.export _int_disabled

	; exported debugging tools
	.export _plt_monitor
	.export _plt_reboot
	.export outchar

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

; Base address of SIO/2 chip 0x80
SIOA_C	.equ	0x80
SIOA_D	.equ	SIOA_C+1
SIOB_C	.equ	SIOA_C+2
SIOB_D	.equ	SIOA_C+3

CTC_CH0	.equ	0x88	; CTC channel 0 and interrupt vector
CTC_CH1	.equ	0x89	; CTC channel 1 (serial B)
CTC_CH2	.equ	0x8A	; CTC channel 2 (timer)
CTC_CH3	.equ	0x8B	; CTC channel 3 (timer count)

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.buffers

	.export _bufpool

_bufpool:
	.ds BUFSIZE * NBUFS

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (kept even when we task switch)
; -----------------------------------------------------------------------------
	.common
;
;	Interrupt flag. This needs to be in common memory for most memory
;	models. It starts as 1 as interrupts start off.
;
_int_disabled:
	.byte 1
;
;	This method is invoked early in interrupt handling before any
;	complex handling is done. It's useful on a few platforms but
;	generally a ret is all that is needed
;
plt_interrupt_all:
	ret

	.code		; so this is above 32K (TODO)
;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
_plt_monitor:
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_plt_reboot:
	di
	ld a, 0x43
	out (CTC_CH0),a
	out (CTC_CH1),a
	out (CTC_CH2),a
	out (CTC_CH3),a
	xor a
	out (0x30),a	;	ROM low
	rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	.code
;
;	This routine is called very early, before the boot code shuffles
;	things into place.
;
init_early:
	ret

;
;	Serial I/O helper for SIO
;
	.export _sio2_otir

_sio2_otir:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	ld	b,0x06
	ld	c,l
	ld	hl,_sio_r
	otir
	pop	bc
	ret


; -----------------------------------------------------------------------------
; DISCARD is memory that will be recycled when we exec init
; -----------------------------------------------------------------------------
	.discard
;
;	After the kernel has shuffled things into place this code is run.
;	It's the best place to breakpoint or trace if you are not sure your
;	kernel is loading and putting itself into place properly.
;
;	It's required jobs are to set up the vectors, ramsize (total RAM),
;	and procmem (total memory free to processs), as well as setting the
;	interrupt mode but *not* enabling interrupts. Many platforms also
;	program up support hardware like PIO and CTC devices here.
;
init_hardware:
	ld hl, 128			; 128K total
	ld (_ramsize), hl
	ld de, 64			; 64 for the kernel bank
	or a				; maybe use ROM for kernel too ?
	sbc hl,de
	ld (_procmem), hl

	; set up interrupt vectors for the kernel

	;
	; Defense in depth - shut everything up first
	;

	ld a, 0x43
	out (CTC_CH0),a			; set CH0 mode
	out (CTC_CH1),a			; set CH1 mode
	out (CTC_CH2),a			; set CH2 mode
	out (CTC_CH3),a			; set CH3 mode

	; now install the interrupt vector at 0x0038

	ld a, 0xC3 ; JP instruction
	ld (0x0038), a
	ld hl, interrupt_handler
	ld (0x0039), hl

	ld (0x0000), a
	ld hl, null_handler   ;   to Our Trap Handler
	ld (0x0001), hl

	;
	; Probe for a CTC
	;

	ld a, 0x47			; CTC 2 as counter
	out (CTC_CH2),a
	ld a, 0xAA			; Set a count
	out (CTC_CH2),a
	in a,(CTC_CH2)
	cp 0xAA
	jr z, maybe_ctc
	cp 0xA9				; Should be one less
	jr nz, no_ctc
maybe_ctc:
	ld a, 0x07
	out (CTC_CH2),a
	ld a, 2
	out (CTC_CH2),a

	; We are now counting down from 2 very fast, so should only see
	; those values on the bus

	ld b, 0
ctc_check:
	in a,(CTC_CH2)
	and 0xFC
	jr nz, no_ctc
	djnz ctc_check

	;
	; Looks like we have a CTC
	;

have_ctc:
	ld a, 1
	ld (_ctc_present),a
	ld (_plt_tick_present),a

	;
	; Set up timer for 200Hz
	;

	ld a, 0xB5
	out (CTC_CH2),a
	ld a, 144
	out (CTC_CH2),a	; 200 Hz

	;
	; Set up counter CH3 for official SIO (the SC110 sadly can't be
	; used this way).

	ld a, 0x47
	out (CTC_CH3),a
	ld a, 255
	out (CTC_CH3),a

no_ctc:
	ld hl, sio_setup
	ld bc, 0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl, sio_setup
	ld bc, 0x0A00 + SIOB_C		; and to SIOB_C
	otir
	im 1 ; set CPU interrupt mode

	ret

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x18
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte 0xEA		; RTS low

;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available.
;
	.commondata

mapreg:    .byte 0	; Our map register is write only so keep a copy
mapsave:   .byte 0	; Saved copy of the previous map (see map_save)

_kernel_flag:
	.byte 1	; We start in kernel mode

;
;	This is invoked with a NULL argument at boot to set the kernel
;	vectors and then elsewhere in the kernel when the kernel knows
;	a bank may need vectors writing to it.
;
_program_vectors:
	; Vectors live in fixed low space
	ret

;
;	Mapping set up for the SC720
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
;
;	The _di versions of the functions are called when we know interrupts
;	are definitely off. In our case it's not useful information so both
;	symbols end up at the same code.
;
map_buffers:
	   ; for us no difference. We could potentially use a low 32K bank
	   ; for buffers but it's not clear it would gain us much value
map_kernel_di:
map_kernel:
map_kernel_restore:		; On banked kernels this is different
				; on unbanked it is just a map_kernel
	push af
	ld a, 0x01		; RAM 0 low 1 high (kernel)
	ld (mapreg),a
	out (0x38), a
	pop af
	ret
	; map_proc is called with HL either NULL or pointing to the
	; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
	ld a, h
	or l
	jr z, map_kernel
map_proc_hl:
	ld a, (hl)			; and fall through
	;
	; With a simple bank switching system you need to provide a
	; method to switch to the bank in A without corrupting any
	; other registers. The stack is safe in common memory.
	; For swap you need to provide what for simple banking is an
	; identical routine.
map_for_swap:
map_proc_a:			; used by bankfork
	ld (mapreg), a	; bank
	out (0x38), a
	ret

	;
	; Map the current process into memory. We do this by extracting
	; the bank value from u_page.
	;
map_proc_always_di:
map_proc_always:
	push af
	ld a, (_udata + U_DATA__U_PAGE)
	ld (mapreg),a
	out (0x38),a
	pop af
	ret

	;
	; Save the existing mapping and switch to the kernel.
	; The place you save it to needs to be in common memory as you
	; have no idea what bank is live. Alternatively defer the save
	; until you switch to the kernel mapping
	;
map_save_kernel:   push af
	ld a, (mapreg)
	ld (mapsave), a
	ld a, 0x01		; Kernel map (see map_kernel)
	ld (mapreg),a
	out (0x38),a
	pop af
	ret
	;
	; Restore the saved bank. Note that you don't need to deal with
	; stacking of banks (we never recursively use save/restore), and
	; that we may well call save and decide not to call restore.
	;
map_restore:
	push af
	ld a, (mapsave)
	ld (mapreg), a
	out (0x38), a
	pop af
	ret

	;
	; Used for low level debug. Output the character in A without
	; corrupting other registers. May block. Interrupts and memory
	; state are undefined
	;
outchar:
	push af
ocloop_sio:
	xor a                   ; read register 0
	out (0x80), a
	in a,(0x80)			; read Line Status Register
	and 0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (0x81),a
	ret

