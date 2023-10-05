;
;	ADAM support
;

        .module adam

        ; exported symbols
        .globl init_hardware
        .globl interrupt_handler
        .globl _program_vectors
	.globl _kernel_flag
	.globl map_page_low
	.globl map_kernel_low
	.globl map_user_low
	.globl map_save_low
	.globl map_restore_low
	.globl _plt_doexec
	.globl _plt_reboot
	.globl _int_disabled
	.globl syscall_platform

        ; exported debugging tools
        .globl _plt_monitor
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl istack_top
        .globl istack_switched_sp
	.globl kstack_top
        .globl unix_syscall_entry
        .globl outcharhex
	.globl outstring

	.globl _vdp_init
	.globl _vdp_load_font
	.globl _vdp_wipe_consoles
	.globl _vdp_restore_font
	.globl _vtinit

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../kernel-z80.def"


;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS

; -----------------------------------------------------------------------------
;
;	Because of the weird memory model this is a bit different to the
;	usual Z80 setup.
;
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	    ; Reboot ends up back in the monitor
_plt_reboot:
	;; TODO: firmware call

_int_disabled:
	.db 1

; -----------------------------------------------------------------------------
;
;	Our MMU is write only, but if we put a value in each bank in a fixed
;	place we have a fast way to see which bank is which
;
; -----------------------------------------------------------------------------

banknum:
	.byte 0x06		; copied into far bank then set to 0x02

; -----------------------------------------------------------------------------
;	All of discard gets reclaimed when init is run
;
;	Discard must be above 0x8000 as we need some of it when the ROM
;	is paged in during init_hardware
; -----------------------------------------------------------------------------
	.area _LOW

init_hardware:
	ld hl, #128
        ld (_ramsize), hl
	ld hl,#60		; We lose 4K to common space
        ld (_procmem), hl

	ld	hl,#s__COMMONMEM
	ld	de,#0x0602
	ld	c,#0x7F
	;	Move COMMON between banks (we are in low 32K)
next_byte:
	ld	a,(hl)
	out	(c),e
	ld	(hl),a
	out	(c),d
	inc	hl
	djnz	next_byte
	ld	a,#0xF4
	cp	h
	jr	nz,next_byte

	;	Back to kernel bank
	out	(c),e

	;	Common set up so can use helper

	ld	hl,#stubs_low
	ld	de,#0
	ld	bc,#0x68
	ldir
	ld	hl,#stubs_low
	ld	ix,#0
	ld	bc,#0x68
	call	ldir_to_user

	call	_vdp_init
	call	_vdp_load_font
	call	_vdp_wipe_consoles
	call	_vdp_restore_font
	; TODO: ensure NMI int is off

	call	_vtinit		; init the console video

	im 1	; set Z80 CPU interrupt mode 1
	ret

;
;	Our memory setup is weird and common is kind of meaningless here
;
	    .area _CODE

_kernel_flag:
	    .db 1	; We start in kernel mode
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

_plt_doexec	.equ	0x18

        .area _DISCARD

	.globl rst38
	.globl stubs_low
;	This exists at the bottom of each page. We move these into place
;	from discard.
;
stubs_low:
	    .byte 0
stub0:	    .word 0		; cp/m emu changes this
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
	    ld a,#0x81
	    out (0x38),a
	    rlca
	    out (0x30),a
	    ei
rst20:	    jp (hl)
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
rst28:	    ret
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
rst30:	    nop
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
rst38:	    jp interrupt_high		; Interrupt handling stub
	    nop
	    nop
	    nop
	    nop
	    nop
	    .ds 0x26
nmi_handler:		; Should be at 0x66
	    push af
	    in a,(0xA0)
	    pop af	; shouldn't happen or be used
	    retn

;
;	This stuff needs to live somewhere, anywhere out of the way (so we
;	use common). We need to copy it to the same address on both banks
;	so place it in common as we will put common into both banks
;

	    .area _COMMONMEM

	    .globl ldir_to_user
	    .globl ldir_from_user
;
;	This needs some properly optimized versions!
;
; FIXME: save a byte and some setup by makign byte 2 of 0to1 the first
; of 1to0
_bank0to1:
	   .dw 0x0001			; 0x0181 for SC108
_bank1to0:
	   .dw 0x0100			; 0x8101 for SC108
_bankport:
	   .db 0x30			; 0x38 for SC108
ldir_to_user:
	    ld de,#0x0206		; from bank 0 to bank  1
ldir_far:
	    push bc
	    ld c,#0x7F
	    exx
	    pop bc			; get BC into alt bank
far_ldir_1:
	    exx
	    out (c),d
	    ld a,(hl)
	    inc hl
	    out (c),e
	    ld (ix),a
	    inc ix
	    exx
	    dec bc
	    ld a,b
	    or c
	    jr nz, far_ldir_1
	    ld a,#0x02
	    out (0x7F),a
	    ret
ldir_from_user:
	    ld de,#0x0602
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
	    ;
	    ; The trick going on here is that an SC108 will respond to
	    ;  0x38 bit 7 = RAM bank 0 = ROM and ignore 0x30
            ; but the SC114 will respond to 0x38 bit 0 only (ROM) and
	    ; to 0x30 bit 0 for RAM.
	    ;
	    ld a,#0x02
	    out (0x7F),a		; Internal, No ROM
	    ld (istack_switched_sp),sp	; istack is positioned to be valid
	    ld sp,#istack_top		; in both banks. We just have to
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
	    ld a,#0x06			; User bank
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
	    ld de,#irqsigret
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
	    ld ix,#0
	    add ix,sp
	    push bc
	    ld c,6(ix)
	    ld b,7(ix)
	    ld e,8(ix)
	    ld d,9(ix)
	    ld l,10(ix)
	    ld h,11(ix)
	    push hl
	    ld l,12(ix)
	    ld h,13(ix)
	    pop ix
	    di
	    ; AF' can be changed in the ABI
	    ex af, af'		; Ick - find a better way to do this bit !
	    ld a,#0x02
	    out (0x7F),a
	    ex af,af'
	    ; Stack now invalid
	    ld (_udata + U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    call unix_syscall_entry
	    ; FIXME check di rules
	    ; stack now invalid. Grab the new sp before we unbank the
	    ; memory holding it
	    ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	    ld a,#0x86		; back to the user page
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
error:	    scf
	    pop bc
	    pop ix
	    ei
	    ret
syscall_sigret:
	    ld a,l		; DEBUG
	    push hl		; save errno
	    push de		; save retval
	    ld l,h
	    ld h,#0
	    push hl		; signal
	    ld hl,#syscall_sighelp
	    push hl		; vector
	    push bc
	    ret
syscall_sighelp:
	    pop de		; discard signal
	    pop de		; recover error info
	    pop hl
	    ld h,#0		; clear signal bit
	    ret


;
;	Adam firmware interface F400-FFFF is the minimal OS segment
;	we keep alive.
;
	.globl _getpcb
	.globl _getdcb
	.globl _lptout
	.globl _lptcheck
	.globl _keypoll
	.globl _keycheck
	.globl _readbegin
	.globl _readdone
	.globl _writebegin
	.globl _writedone

	.globl _andev
	.globl _lbalo
	.globl _lbahi


_getpcb:
	push	iy
	call	0xFC5A
	push	iy
	pop	hl
	pop	iy
	ret

_getdcb:	; z88dk fastcall
	push	iy
	ld	a,l
	call	0xFC5A
	push	iy
	pop	hl
	pop	iy
	ret

_lptout:	; z88dk fastcall
	ld	a,l	; char
	call	0xFC69F
	ld 	l,a
	ret
_lptcheck:
	call	0xFC42
	ld	l,#0
	jr	nz,lpterr
	ret	c
	inc	l		; 1 not completed
	ret
lpterr:
	dec	l		; -1 error
	ret	

_keypoll:
	call	0xFCA8
	ld 	a,l
	ret
_keycheck:
	call	0xFC4B
	ld	l,#-1
	ret	nz
	ld	l,a
	ret	c
	ld	l,#0
	ret
_readbegin:	; __z88dk_fastcall of data (kernel)
	ld	a,(_andev)
	ld	bc,(_lbahi)
	ld	de,(_lbalo)
	call	0xFCA2
	ld	l,a
	ret	nz
	ld	l,#0
	ret
_readdone:
	ld	a,(_andev)
	call	0xFC45
	ld	l,#0
	ret	nc
	ld	l,#1
	ret	z
	ld	l,a
	ret	

_writebegin:	; __z88dk_fastcall of data (kernel)
	ld	a,(_andev)
	ld	bc,(_lbahi)
	ld	de,(_lbalo)
	call	0xFCAB
	ld	l,a
	ret	nz
	ld	l,#0
	ret
_writedone:
	ld	a,(_andev)
	call	0xFC4E
	ld	l,#0
	ret	nc
	ld	l,#1
	ret	z
	ld	l,a
	ret	
_reboot:
	; need to reload EOS from ROM first (or just map OS7 and rst 0 ?)
	jp	0xFC30
