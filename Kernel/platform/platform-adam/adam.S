;
;	ADAM support
;
;	The fully loaded ADAM has 128K of RAM. We can control the upper and
;	lower 32K independently.
;
;	Lower 32K
;	- Intrinsic lower 32K (used for kernel, DMAable for I/O)
;	- Expansion lower 32K (used for user)
;
;	Upper 32K
;	- Intrinsic upper 32K (used for kernel,, DMAable for I/O)
;	- Expansion upper 32K (used for user)
;
;
;	The fancier expansion boards used port 0x42 to select between
;	multiple 64K banks of "expansion". We don't deal with this yet.
;	These went up to 2MB
;

        ; exported symbols
        .export init_hardware
        .export interrupt_handler
        .export _program_vectors
	.export _kernel_flag
	.export map_page_low
	.export map_kernel_low
	.export map_user_low
	.export map_save_low
	.export map_restore_low
	.export _plt_doexec
	.export _plt_reboot
	.export _int_disabled
	.export syscall_platform

        ; exported debugging tools
        .export _plt_monitor
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
;
;	Because of the weird memory model this is a bit different to the
;	usual Z80 setup.
;
; -----------------------------------------------------------------------------
	.common
_plt_monitor:
	; Reboot ends up back in the monitor
_plt_reboot:
	;; TODO: firmware call

_int_disabled:
	.byte 1

; -----------------------------------------------------------------------------
;
;	Our MMU is write only, but if we put a value in each bank in a fixed
;	place we have a fast way to see which bank is which
;
; -----------------------------------------------------------------------------

banknum:
	.byte 0x0A		; copied into far bank then set to 0x01

; -----------------------------------------------------------------------------
;	All of discard gets reclaimed when init is run
;
;	Discard must be above 0x8000 as we need some of it when the ROM
;	is paged in during init_hardware
; -----------------------------------------------------------------------------
	.discard		; low ??

init_hardware:
	ld hl,	128
        ld (_ramsize), hl
	ld hl,	60		; We lose 4K to common space
        ld (_procmem), hl

	ld	hl, __common
	ld	de, 0x0A01
	ld	c, 0x7F
	;	Move COMMON between banks (we are in low 32K)
next_byte:
	ld	a,(hl)
	out	(c),e
	ld	(hl),a
	out	(c),d
	inc	hl
	djnz	next_byte
	ld	a, 0xF3
	cp	h
	jr	nz,next_byte

	;	Back to kernel bank
	out	(c),e

	;	Common set up so can use helper

	ld	hl, stubs_low
	ld	de, 0
	ld	bc, 0x68
	ldir
	ld	hl, stubs_low
	ld	ix, 0
	ld	bc, 0x68
	call	ldir_to_user

	call	_vdp_init
	call	_vdp_load_font
	call	_vdp_wipe_consoles
	call	_vdp_restore_font
	; TODO: ensure NMI int is off

	call	_vtinit		; init the console video

	im	1	; set Z80 CPU interrupt mode 1
	ret

;
;	Our memory setup is weird and common is kind of meaningless here
;
	.area _CODE

_kernel_flag
	.byte	1	; We start in kernel mode
map_save_low:
map_kernel_low:
map_restore_low:
map_user_low:
map_page_low:
	ret

_program_vectors:
	ret

;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
outchar:
	ret


; Don't be tempted to put the symbol in the code below ..it's relocated
; to zero. Instead define where it ends up.

	.abs
	.org 0x0018

_plt_doexec:

	.discard

	.export rst38
	.export stubs_low
;	This exists at the bottom of each page. We move these into place
;	from discard.
;
stubs_low:
	.byte 0
stub0:
	.word 0		; cp/m emu changes this
	.byte 0		; cp/m emu I/O byte
	.byte 0		; cp/m emu drive and user
	jp 0		; cp/m emu bdos entry point
rst8:
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst10:
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst18:
	ld a, 0x0A	;	TODO bank flip
	out (0x7F),a
	ei
	jp (hl)
	nop
	nop
rst20:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst28:
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst30:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
;
;	We only have 38-4F available for this in low space
;
rst38:
	jp interrupt_high		; Interrupt handling stub
	nop
	nop
	nop
	nop
	nop
	.ds 0x26
nmi_handler:		; Should be at 0x66
	push af
	in a,(0xBE)
	pop af	; shouldn't happen or be used
	retn

;
;	This stuff needs to live somewhere, anywhere out of the way (so we
;	use common). We need to copy it to the same address on both banks
;	so place it in common as we will put common into both banks
;

	.common

	.export ldir_to_user
	.export ldir_from_user
;
;	This needs some properly optimized versions!
;
ldir_to_user:
	ld de, 0x010A		; from bank 0 to bank  1
ldir_far:
	push bc
	exx
	pop bc			; get BC into alt bank
far_ldir_1:
	exx
	out (0x7F),d
	ld a,(hl)
	inc hl
	out (0x7F),e
	ld (ix),a
	inc ix
	exx
	dec bc
	ld a,b
	or c
	jr nz, far_ldir_1
	ld a, 0x01
	out (0x7F),a
	ret
ldir_from_user:
	ld de, 0x0A01
	jr ldir_far
;
;	High stubs. Present in each bank in the top 256 bytes
;
interrupt_high:
	push af
	push de
	push hl
	ex af,af'
	push af
	push bc
	exx
	push bc
	push de
	push hl
	push ix
	push iy
	ld a,(banknum)
	ld c,a

	ld a, 0x01
	out (0x7F),a			; Internal, No ROM
	ld (istack_switched_sp),sp	; istack is positioned to be valid
	ld sp, istack_top		; in both banks. We just have to
	;
	;	interrupt_handler may come back on a different stack in
	;	which case bc is junk. Fortuntely we never pre-empt in
	;	kernel so the case we care about bc is always safe. This is
	;	not a good way to write code and should be fixed! FIXME
	;
	push bc
	call interrupt_handler	; switch on the right SP
	pop bc
	; Restore stack pointer to user. This leaves us with an invalid
	; stack pointer if called from user but interrupts are off anyway
	ld sp,(istack_switched_sp)
	; On return HL = signal vector E= signal (if any) A = page for
	; high
	or a
	jr z, kernout
	; Returning to user space
	ld a, 0x0A			; User bank
	out (0x7F),a
	; User stack is now valid
	; back on user stack
	xor a
	cp e
	call nz, sigpath
pops:
	ex af,af'
	exx
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	exx
	pop bc
	pop af
	ex af,af'
	pop hl
	pop de
	pop af
	ei
	ret
kernout:
	; restore bank - if we interrupt mid user copy or similar we
	; have to put the right bank back
	ld a,c
	out (0x7F),a
	jr pops

sigpath:
	push de		; signal number
	ld de, irqsigret
	push de		; clean up
	ex de,hl		; move the vector into DE
	ld hl,(PROGLOAD+16)	; helper pointer
	jp (hl)		; into helper
irqsigret:
	inc sp		; drop signal number
	inc sp
	ret
;
;	Our stack looks like this when we start accessing arguments
;
;	12	arg3
;	10	arg2
;	8	arg1
;	6	arg0
;	4	user address
;	2	syscall return to user address
;	0	ix
;
;	and A holds the syscall number
;
syscall_platform:
	push ix
	ld ix, 0
	add ix,sp
	push bc
	ld c,(ix + 6)
	ld b,(ix + 7)
	ld e,(ix + 8)
	ld d,(ix + 9)
	ld l,(ix + 10)
	ld h,(ix + 11)
	push hl
	ld l,(ix + 12)
	ld h,(ix + 13)
	pop ix
	di
	; AF' can be changed in the ABI
	ex af, af'		; Ick - find a better way to do this bit !
	ld a, 0x01
	out (0x7F),a
	ex af,af'
	; Stack now invalid
	ld (_udata + U_DATA__U_SYSCALL_SP),sp
	ld sp, kstack_top
	call unix_syscall_entry
	; FIXME check di rules
	; stack now invalid. Grab the new sp before we unbank the
	; memory holding it
	ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	ld a, 0x0A		; back to the user page
	out (0x7F),a
	xor a
	cp h
	call nz, syscall_sigret
	; FIXME for now do the grungy C flag HL DE stuff from
	; lowlevel-z80 until we fix the ABI
	ld a,h
	or l
	jr nz, error
	ex de,hl
	pop bc
	pop ix
	ei
	ret
error:
	scf
	pop bc
	pop ix
	ei
	ret
syscall_sigret:
	ld a,l		; DEBUG
	push hl		; save errno
	push de		; save retval
	ld l,h
	ld h, 0
	push hl		; signal
	ld hl, syscall_sighelp
	push hl		; vector
	push bc
	ret
syscall_sighelp:
	pop de		; discard signal
	pop de		; recover error info
	pop hl
	ld h, 0		; clear signal bit
	ret


;
;	Adam firmware interface F400-FFFF is the minimal OS segment
;	we keep alive.
;
	.export _getpcb
	.export _getdcb
	.export _lptout
	.export _lptcheck
	.export _keypoll
	.export _keycheck
	.export _readbegin
	.export _readdone
	.export _writebegin
	.export _writedone

	.export _andev
	.export _lbalo
	.export _lbahi


_getpcb:
	push	bc
	push	ix
	push	iy
	call	0xFC5A
	push	iy
	pop	hl
	pop	iy
	pop	ix
	pop	bc
	ret

_getdcb:	; z88dk fastcall
	push	bc
	push	ix
	push	iy
	ld	a,l
	call	0xFC5A
	push	iy
	pop	hl
	pop	iy
	pop	ix
	pop	bc
	ret

_lptout:
	pop	de
	pop	hl
	push	hl
	push	de
	ld	a,l	; char
	push	bc
	push	ix
	push	iy
	call	0xFC69F
	pop	iy
	pop	ix
	pop	bc
	ld 	l,a
	ret
_lptcheck:
	push	bc
	push	ix
	push	iy
	call	0xFC42
	pop	iy
	pop	ix
	pop	bc
	ld	l, 0
	jr	nz,lpterr
	ret	c
	inc	l		; 1 not completed
	ret
lpterr:
	dec	l		; -1 error
	ret	

_keypoll:
	push	bc
	push	ix
	push	iy
	call	0xFCA8
	pop	iy
	pop	ix
	pop	bc
	ld 	l,a
	ld	h,0
	ret
_keycheck:
	push	bc
	push	ix
	push	iy
	call	0xFC4B
	pop	iy
	pop	ix
	pop	bc
	ld	hl, -1
	ret	nz
	ld	h,0
	ld	l,a
	ret	c
	ld	l, 0
	ret
_readbegin:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	push	ix
	push	iy
	ld	a,(_andev)
	ld	bc,(_lbahi)
	ld	de,(_lbalo)
	call	0xFCA2
	pop	iy
	pop	ix
	pop	bc
	ld	l,a
	ret	nz
	ld	l, 0
	ret
_readdone:
	ld	a,(_andev)
	push	bc
	push	ix
	push	iy
	call	0xFC45
	pop	iy
	pop	ix
	pop	bc
	ld	hl, 0
	ret	nc
	ld	l, 1
	ret	z
	ld	l,a
	ret	

_writebegin:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	push	ix
	push	iy
	ld	a,(_andev)
	ld	bc,(_lbahi)
	ld	de,(_lbalo)
	call	0xFCAB
	pop	iy
	pop	ix
	pop	bc
	ld	l,a
	ret	nz
	ld	l, 0
	ret
_writedone:
	ld	a,(_andev)
	push	bc
	push	ix
	push	iy
	call	0xFC4E
	pop	iy
	pop	ix
	pop	bc
	ld	hl, 0
	ret	nc
	ld	l, 1
	ret	z
	ld	l,a
	ret	
_reboot:
	; need to reload EOS from ROM first (or just map OS7 and rst 0 ?)
	jp	0xFC30
