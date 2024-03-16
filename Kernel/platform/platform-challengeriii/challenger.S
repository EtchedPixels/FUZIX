;
;	Challenger III support
;
;	Note that this system is entirely MMIO
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

	.common

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
	; Set up for bank switching
	ld hl,0xF701
	ld (hl),4
	ld a,0x0F
	ld (0xF700),a

	ld a,0x0E		; 0x0F is present defacto
	ld hl,0x0004		; safe address for poking

	ld de,1			; 0xF is present
next:
	rl e
	rl d
	ld (0xF700),a		; bank set
	ld (hl),a
	cp (hl)
	jr nz, bad
	inc (hl)
	cp (hl)
	jr z, bad
	dec (hl)
	cp (hl)
	jr nz, bad
	inc e
bad:
	dec a
	jr nz, next

	ld a,15			; restore system memory mapping
	ld (0xF700),a

	; DE is now a bit mask of present banks
	ld (_bankmask),de

	ld b,16
	xor a
countbits:
	rl e
	rl d
	adc a,0
	djnz countbits

	; A is now number of 1 bits (each being 48K) and max of 16
	; Convert into KB
	add a,a
	add a,a
	add a,a		; x 8 then might overflow
	ld l,a
	ld h,0
	add hl,hl	; x 16
	push hl
	add hl,hl	; x 32
	pop de
	add hl,de	; x 48

	ld de,#8
	add hl,de	; 8K at Dxxx

	ld (_ramsize), hl
	ld de,#56		; and 56K for kernel
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

	; Divide by 16, RTS low, RX interrupts 8N2 */
	ld a,0x91
	ld (0xFC00),a
	ret

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
	push	af
	ld	a,0x0F
	ld	(mapreg),a
	ld	(0xF700),a
	pop	af
	ret
	; map_proc is called with HL either NULL or pointing to the
	; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
	ld	a, h
	or	l
	jr	z, map_kernel
map_proc_hl:
	ld	a, (hl)			; and fall through
	;
	; With a simple bank switching system you need to provide a
	; method to switch to the bank in A without corrupting any
	; other registers. The stack is safe in common memory.
	; For swap you need to provide what for simple banking is an
	; identical routine.
map_for_swap:
map_proc_a:			; used by bankfork
	ld	(mapreg), a	; bank
	ld	(0xF700), a
	ret

	;
	; Map the current process into memory. We do this by extracting
	; the bank value from u_page.
	;
map_proc_always_di:
map_proc_always:
	push	af
	ld	a,(_udata + U_DATA__U_PAGE)
	ld	(mapreg),a
	ld	(0xF700),a
	pop	af
	ret

	;
	; Save the existing mapping and switch to the kernel.
	; The place you save it to needs to be in common memory as you
	; have no idea what bank is live. Alternatively defer the save
	; until you switch to the kernel mapping
	;
map_save_kernel:
	push af
	ld	a, (mapreg)
	ld	(mapsave), a
	ld	a,0x0F
	ld	(mapreg),a
	ld	(0xF700),a
	pop	af
	ret
	;
	; Restore the saved bank. Note that you don't need to deal with
	; stacking of banks (we never recursively use save/restore), and
	; that we may well call save and decide not to call restore.
	;
map_restore:
	push	af
	ld	a, (mapsave)
	ld	(mapreg), a
	ld	(0xF700), a
	pop	af
	ret

	;
	; Used for low level debug. Output the character in A without
	; corrupting other registers. May block. Interrupts and memory
	; state are undefined
	;
outchar:
	push af
twait:	ld a,(0xFC00)
	and #0x02
	jr z, twait
	pop af
	ld (0xFC01),a
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
;	OSI disk helpers for HD
;
	.export _osihd_read
	.export _osihd_write
	.export _osihd_copy_out
	.export _osihd_copy_in

osihd_initcp:
	ld	d,(hl)		; Block offset (low byte)
	inc	hl
	inc	hl
	ld	a,(hl)
	inc	hl
	ld	h,(hl)		; Data pointer
	ld	l,a
	ld	e,0
	ex	de,hl
	; Block offset (in 256s) in HL, dptr in DE
	add	hl,hl		; 512s
	ld	bc,0xE018	; Disk buffer (TODO CD7 this moves)
	add	hl,bc		; offset
	ld	a,(_td_raw)
	or	a
	ret	z
	dec	a
	jp	z,map_proc_always
	ld	a,(_td_page)
	jp	map_for_swap

wait_seek:
	ld	a,(0xC202)
	cp	0xD9
	ret	z
	and	0xC4
	jr	nz,wait_seek
	inc	a	; force NZ
	ret

_osihd_copy_out:
	ld	hl,2
	add	hl,sp
	push	bc
	call	osihd_initcp
docopy:
	ld	bc,512
	; Unrolled for a bit more speed. At 1Mhz every little helps
	; especially as all disk I/O hits this
	; Do 32 LDI. Each will dec bc
copyout:
	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	ldi
	ldi
	ldi
	ldi

	jp	pe, copyout

	pop	bc
	jp	map_kernel

_osihd_copy_in:
	ld	hl,2
	add	hl,sp
	push	bc
	call	osihd_initcp
	ex	de,hl
	jr	docopy

	; TODO: Push down DI stuff so we don't di during the seek phase
_osihd_write:
	push	bc
	ld	b,0xC0
	jr	osihd_op
_osihd_read:
	push	bc
	ld	b,0x80
osihd_op:
	ld	hl,0xC280
	ld	de,(_hddat);	; head in E cyl in D
	ld	a,(hl)
	ld	(0xC200),de
	ld	a,0x10
	ld	c,(hl)
	ld	(0xC202),a	; Seek please
	xor	a
	ld	(0xC202),a
	; Wait for seek completion
	call	wait_seek
	jr	nz, failed
	ld	de,(_hddat + 2)
	ld	a,(hl)
	ld	(0xC203),de
	ld	de,(_hddat + 4)
	ld	a,(hl)
	ld	(0xC205),de
	ld	a,b
	ld	e,(hl)
	ld	(0xC207),a		; Issue command
wait_done:
	ld	a,(0xC207)
	rla
	jr	c, wait_done
	ld	hl,1
	pop	bc
	ret
failed:
	ld	hl,0
	pop	bc
	ret

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
	;	PPIDE over a PIA
	ld	hl,0xCD02
rdloop:
	ld	(hl),0xD0	; /RD low
	ld	a,(0xCD00)
	ld	(hl),0xF0	; RD back high
	ld	(de),a
	inc	de
	ld	(hl),0xD0	; /RD low
	ld	a,(0xCD00)
	ld	(hl),0xF0	; RD back high
	ld	(de),a
	inc	de
	djnz	rdloop
	pop	bc
	jp	map_kernel_restore

_devide_write_data:
	call	_ide_set_w
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	ide_map
wrloop:
	ld	a,(de)
	ld	(0xCD00),a
	ld	(hl),0xD0	; /WR low
	inc	de
	ld	(hl),0xF0	; WR back high
	ld	a,(de)
	ld	(0xCD00),a
	ld	(hl),0xD0	; /WR low
	inc	de
	ld	(hl),0xF0	; WR back high
	djnz	rdloop
	pop	bc
	call	map_kernel_restore
	jp	_ide_set_r
