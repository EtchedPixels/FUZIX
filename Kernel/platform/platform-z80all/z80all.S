;
;	Z80ALL support
;
	; exported symbols
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

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.export _bufpool

	.buffers

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
	halt

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	.code

;
;	This routine is called very early, before the boot code shuffles
;	things into place. We do the ttymap here mostly as an example but
;	even that really ought to be in init_hardware.
;
init_early:
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
	ld hl,#128
	ld (_ramsize), hl
	ld de,#64		; and 64K for kernel (so 128K gets you only
				; two user banks which is OK single user)
	or a
	sbc hl,de
	ld (_procmem), hl

	; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
	ld hl, #0
	push hl
	call _program_vectors
	pop hl

	; Install the rst block for shortening code size
	ld hl,#rstblock
	ld de,#8
	ld bc,#32
	ldir

	im 1 ; set CPU interrupt mode

	; 60Hz timer interrupt enable
	ld a,0x40
	out (0xF5),a

	jp _video_init
;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available.
;
	.common

mapreg:
	.byte 0	; Our map register is write only so keep a copy
mapsave:
	.byte 0	; Saved copy of the previous map (see map_save)

_kernel_flag:
	.byte 1	; We start in kernel mode

;
;	This is invoked with a NULL argument at boot to set the kernel
;	vectors and then elsewhere in the kernel when the kernel knows
;	a bank may need vectors writing to it.
;
_program_vectors:
	; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	di ; just to be sure
	pop de ; temporarily store return address
	pop hl ; function argument -- base page number
	push hl ; put stack back as it was
	push de

	call map_proc

	; now install the interrupt vector at 0x0038
	ld a, #0xC3 ; JP instruction
	ld (0x0038), a
	ld hl, #interrupt_handler
	ld (0x0039), hl

	; set restart vector for FUZIX system calls
	ld (0x0030), a   ;  (rst 30h is unix function call vector)
	ld hl, #unix_syscall_entry
	ld (0x0031), hl

	ld (0x0000), a
	ld hl, #null_handler   ;   to Our Trap Handler
	ld (0x0001), hl
#if 0
	; RST peepholes support
	ld hl,#___sdcc_enter_ix
	ld (0x08),a
	ld (0x09),hl
	ld hl,#___spixret
	ld (0x10),a
	ld (0x11),hl
	ld hl,#___ixret
	ld (0x18),a
	ld (0x19),hl
	ld hl,#___ldhlhl
	ld (0x20),a
	ld (0x21),hl
#endif


	; and fall into map_kernel

;
;	Mapping set up for the SBCv2
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
;
;	We know the ROM mapping is already off
;
;	The _di versions of the functions are called when we know interrupts
;	are definitely off. In our case it's not useful information so both
;	symbols end up at the same code.
;
map_buffers:
	   ; for us no difference. We could potentially use a low 32K bank
	   ; for buffers but it's not clear it would gain us much value
	   ; 1F always the top 32K
map_kernel_restore:
map_kernel_di:
map_kernel:
	push af
	xor a
	ld (mapreg),a
	out (0x1F), a
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
	out (0x1F), a
	ret

	;
	; Map the current process into memory. We do this by extracting
	; the bank value from u_page.
	;
map_proc_always_di:
map_proc_always:
	push af
	ld a,(_udata + U_DATA__U_PAGE)
	ld (mapreg),a
	out (0x1F),a
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
	xor a
	ld (mapreg),a
	out (0x1F),a
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
	out (0x1F), a
	pop af
	ret

outchar:
	; TODO
	ret

	.discard
;
;	This block is copied into the rst area
;

;
;       Stub helpers for code compactness.
;
;   Note that sdcc_enter_ix is in the standard compiler support already
;
;   The first two use an rst as a jump. In the reload sp case we don't
;   have to care. In the pop ix case for the function end we need to
;   drop the spare frame first, but we know that af contents don't
;   matter
;

; 8
rstblock:
#if 0
/* TODO */
	jp	___sdcc_enter_ix
	.ds	5
; 16
___spixret:
	ld      sp,ix
	pop     ix
	ret
	.ds	3
___ixret:
	pop     af
	pop     ix
	ret
	.ds	4
___ldhlhl:
	ld      a,(hl)
	inc     hl
	ld      h,(hl)
	ld      l,a
	ret
#endif

	.common
;
;	IDE disk helpers
;
	.export _devide_read_data
	.export _devide_write_data

ide_map:
	ld	a,(_td_raw)
	ld	bc,0x10		; port 10, 256 times
	or	a
	jp	z, map_buffers
	dec	a
	jp	z, map_proc_always
	ld	a,(_td_page)
	jp	map_for_swap

_devide_read_data:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	ide_map
	inir
	inir
	pop	bc
	jp	map_kernel_restore

_devide_write_data:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	ide_map
	otir
	otir
	pop	bc
	jp	map_kernel_restore
