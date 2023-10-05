;
;	SC108 Support
;

        .module sc108

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
	.globl _acia_present
	.globl _ctc_present
	.globl _sio_present
	.globl _sio1_present
	.globl outstring
	.globl _is_sc108

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../kernel-z80.def"


; Base address of SIO/2 chip 0x80

SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_C+1
SIOB_C		.EQU	SIOA_C+2
SIOB_D		.EQU	SIOA_C+3

SIOC_C		.EQU	0x84
SIOC_D		.EQU	SIOC_C+1
SIOD_C		.EQU	SIOC_C+2
SIOD_D		.EQU	SIOC_C+3

ACIA_C          .EQU     0x80
ACIA_D          .EQU     0x81
ACIA_RESET      .EQU     0x03
ACIA_RTS_HIGH_A      .EQU     0xD6   ; rts high, xmit interrupt disabled
ACIA_RTS_LOW_A       .EQU     0x96   ; rts low, xmit interrupt disabled
;ACIA_RTS_LOW_A       .EQU     0xB6   ; rts low, xmit interrupt enabled


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
	xor a
	out (0x38), a		; ROM appears low
	rst 0			; bang

_int_disabled:
	.db 1

; -----------------------------------------------------------------------------
;
;	Our MMU is write only, but if we put a value in each bank in a fixed
;	place we have a fast way to see which bank is which
;
; -----------------------------------------------------------------------------

banknum:
	.byte 0x81		; copied into far bank then set to 1

; -----------------------------------------------------------------------------
;	All of discard gets reclaimed when init is run
;
;	Discard must be above 0x8000 as we need some of it when the ROM
;	is paged in during init_hardware
; -----------------------------------------------------------------------------
	.area _DISCARD

init_hardware:
	ld hl, #128
        ld (_ramsize), hl
	ld hl,#60		; We lose 4K to common space
        ld (_procmem), hl

	;
	;	Look for an ACIA
	;
	in a,(ACIA_C)
	bit 1,a
	jr z, try_sio
	ld a,#ACIA_RESET
	out (ACIA_C),a
	; TX should now have gone
	in a,(ACIA_C)
	bit 1,a
	jr nz, try_sio
	;	Set up the ACIA

        ld a, #ACIA_RTS_LOW_A
        out (ACIA_C),a         		; Initialise ACIA
	ld a,#1
	ld (_acia_present),a
	jp serial_up


try_sio:
	; Play guess the serial port
	;
	; This could be the ACIA control port. If so we mash the settings
	; up but that is ok as we will put them back in the SIO probe
	;

	xor a
	ld c,#SIOA_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, not_sio_either

	; Repeat the check on SIO B

	xor a
	ld c,#SIOB_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr nz, is_sio

	; Now sanity check the vector register

	ld a,#2
	out (c),a
	ld a,#0xA0
	out (c),a
	ld a,#2
	out (c),a
	in a,(c)
	and #0xF0
	cp #0xA0
	jr z, is_sio

	ld a,#2
	out (c),a
	ld a,#0x50
	out (c),a
	ld a,#2
	out (c),a
	in a,(c)
	and #0xF0
	cp #0x50
	jr z, is_sio
 

	;
	; Doomed I say .... doomed, we're all doomed
	;
	; At least until RC2014 grows a nice keyboard/display card!
	;
	; Fall through and pray
	;
not_sio_either:
;
;	We have an SIO so do the required SIO hdance
;
is_sio:	ld a,#1
	ld (_sio_present),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

	xor a
	ld c,#SIOC_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, serial_up

	; Repeat the check on SIO B
	; We have to be careful here because it could be that this is
	; a mirror of the first SIO! Fortunately channel B WR2 can be read
	; as RR2 so we can write the vector to one and check the other, then
	; write a different vector and check again. If they both change
	; it's aliased, if not it really is two interfaces.

	ld a,#2
	out (SIOB_C),a			; vector
	ld a,#0xF0
	out (SIOB_C),a			; vector is now 0xFX
	ld a,#2
	out (SIOD_C),a
	in a,(SIOD_C)			; read it back on the second SIO
	and #0xF0
	cp #0xF0
	jr nz, not_mirrored		; it's not a mirror, might not be an SIO

	; Could be chance or a soft boot

	ld a,#2
	out (SIOB_C),a
	xor a
	out (SIOB_C),a
	ld a,#2
	out (SIOD_C),a
	in a,(SIOD_C)
	and #0xF0
	jr z, serial_up			; It's a mirage

not_mirrored:
	xor a
	ld c,#SIOD_C
	out (c),a			; RR0
	in b,(c)			; Save RR0 value
	inc a
	out (c),a			; RR1
	in a,(c)
	cp b				; Same from both reads - not an SIO

	jr z, serial_up

	ld a,#0x01
	ld (_sio1_present),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOC_C		; 10 bytes to SIOC_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOD_C		; and to SIOD_C
	otir


serial_up:

	xor a			; Kernel + ROM (works on SC108 and SC114)
	out (0x38),a		;

	; We'd like to just use the ROM helper but unfortunately the boot
	; arrangements fry the ROM stuff. Instead we just find the code
	; we need and then run it.

	ld hl,#0x0100

nextscan:
	ld a,(hl)
	inc hl
	cp #0xED
	jr nz,nextbyte
	ld a,(hl)
	cp #0x41
	jr nz,nextbyte
	inc hl
	ld a,(hl)
	cp #0x12
	jr nz,nextbyte
	inc hl
	ld a,(hl)
	cp #0x05
	jr z, found_it
	cp #0x06
	jr z,found_it
	dec hl
nextbyte:
	bit 6,h
	jr z,nextscan

	ld a,#0x01
	out (0x38),a
	ld hl,#oldrom
	call outstring

	; Patch common before we copy it
	ld de,#0x0181
	ld (_bank0to1),de
	ld de,#0x8101
	ld (_bank1to0),de
	ld a,#0x38
	ld (_bankport),a

	; try the old ROM hack instead

	xor a
	out (0x38),a
	ld hl,#s__COMMONMEM
	ld ix,#s__COMMONMEM
	ld bc,#l__COMMONMEM
	ld de,#0x8000
	xor a
	call 0x7ffd
	; and continue... hopefully
	jr old_sc108

oldrom:
	.asciz 'Bank helper not found, old ROM ?\r\n'

callhl:
	jp (hl)

	; Found an SC108 or SC114
found_it:
	or a
	ld de,#7
	sbc hl,de

	sub #5
	ld (_is_sc108),a	; 0 = 114 1 = 108
	jr z, sc114

	;
	;	Set banking registers for SC108 (they default to SC114)
	;	Must do this before copying as we need it patched in
	;	both banks
	;

	ld de,#0x0181
	ld (_bank0to1),de
	ld de,#0x8101
	ld (_bank1to0),de
	ld a,#0x38
	ld (_bankport),a

sc114:
	ld de,#s__COMMONMEM
	ld bc,#l__COMMONMEM

copynext:
	push bc
	ld a,(de)
	call callhl		; writes by to DE in far bank, eats BC
	inc de
	pop bc
	dec bc
	ld a,b
	or c
	jr nz, copynext

old_sc108:
	ld a,#0x01		; bank 0 ROM out
	out (0x38),a
	ld (banknum),a		; and correct page

	; We now have our common in place. We can do the rest ourselves

	; Put the low stubs into place in the kernel
	ld hl,#stubs_low
	ld de,#0
	ld bc,#0x68
	ldir
	ld hl,#stubs_low
	ld ix,#0
	ld bc,#0x68
	call ldir_to_user

        ; ---------------------------------------------------------------------
	; Initialize CTC
	;
	; Need to do autodetect on this
	;
	; We must initialize all channels of the CTC. The documentation
	; states that the initial CTC state is undefined and we don't want
	; random interrupt surprises
	;
	; ---------------------------------------------------------------------

	;
	; Defense in depth - shut everything up first
	;

	ld a,#0x43
	out (CTC_CH0),a			; set CH0 mode
	out (CTC_CH1),a			; set CH1 mode
	out (CTC_CH2),a			; set CH2 mode
	out (CTC_CH3),a			; set CH3 mode

	;
	; Probe for a CTC
	;

	ld a,#0x47			; CTC 2 as counter
	out (CTC_CH2),a
	ld a,#0xAA			; Set a count
	out (CTC_CH2),a
	in a,(CTC_CH2)
	cp #0xAA
	jr z, maybe_ctc
	cp #0xA9			; Could be one lower
	jr nz, no_ctc
maybe_ctc:
	ld a,#0x07
	out (CTC_CH2),a
	ld a,#2
	out (CTC_CH2),a

	; We are now counting down from 2 very fast, so should only see
	; those values on the bus

	ld b,#0
ctc_check:
	in a,(CTC_CH2)
	and #0xFC
	jr nz, no_ctc
	djnz ctc_check

	;
	; Looks like we have a CTC
	;

have_ctc:
	ld a,#1
	ld (_ctc_present),a

	;
	; Set up timer for 200Hz
	;

	ld a,#0xB5
	out (CTC_CH2),a
	ld a,#144
	out (CTC_CH2),a	; 200 Hz

	;
	; Set up counter CH3 for official SIO (the SC110 sadly can't be
	; used this way).

	ld a,#0x47
	out (CTC_CH3),a
	ld a,#255
	out (CTC_CH3),a

no_ctc:
        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	im 1				; set Z80 CPU interrupt mode 1
	ret

RTS_LOW	.EQU	0xEA

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
	.byte RTS_LOW


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
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld b,#0x06
	ld c,l
	ld hl,#_sio_r
	otir
	ret
;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
outchar:
	push af
	ld a,(_acia_present)
	or a
	jr z, ocloop_sio

ocloop_acia:
	in a,(ACIA_C)
	and #0x02
	jr z, ocloop_acia
	pop af
	out (ACIA_D),a
	ret
	; wait for transmitter to be idle
ocloop_sio:
        xor a                   ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)		; read Line Status Register
	and #0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (SIOA_D),a
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
rst30:	    jp syscall_platform
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
	    ld de,(_bank0to1)		; from bank 0 to bank  1
ldir_far:
	    push bc
	    ld bc,(_bankport)
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
	    ld a,(_bank0to1+1)
	    exx
	    out (c),a
	    exx
	    ret
ldir_from_user:
	    ld de,(_bank1to0)
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
	    ld a,#0x01
	    out (0x38),a		; Bank 0, no ROM
	    rlca
	    out (0x30),a
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
	    ld a,#0x81			; User bank
	    out (0x38),a
	    rlca
	    out (0x30),a
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
	    out (0x38),a
	    rlca
	    out (0x30),a
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
	    ld a,#1
	    out (0x38),a
	    rlca
	    out (0x30),a
	    ex af,af'
	    ; Stack now invalid
	    ld (_udata + U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    call unix_syscall_entry
	    ; FIXME check di rules
	    ; stack now invalid. Grab the new sp before we unbank the
	    ; memory holding it
	    ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	    ld a,#0x81		; back to the user page
	    out (0x38),a
	    rlca
	    out (0x30),a
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

