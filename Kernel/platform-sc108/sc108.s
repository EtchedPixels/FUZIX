;
;	SC108 Initial Support
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
	.globl _platform_doexec
	.globl _platform_reboot
	.globl _int_disabled

        ; exported debugging tools
        .globl _platform_monitor
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl istack_top
        .globl istack_switched_sp
	.globl kstack_top
        .globl unix_syscall_entry
        .globl outcharhex
	.globl _ser_type

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../kernel.def"


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

_platform_monitor:
	    ; Reboot ends up back in the monitor
_platform_reboot:
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
	ld hl,#64
        ld (_procmem), hl

	ld hl,#s__COMMONMEM
	ld ix,#s__COMMONMEM
	ld bc,#l__COMMONMEM
	xor a			; Kernel + ROM
	out (0x38),a		;

	ld de,#0x8000		; bank 0 to bank 1 (ROM in)
	xor a			; return to bank 0
	call 0x7FFD		; ROM helper vector

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

	; Play guess the serial port
	; This needs doing better. We might be fooled by floating flow
	; control lines as the SC104 does expose flow control. FIXME
	in a,(SIOA_C)
	and #0x2C
	cp #0x2C
	; CTS and DCD should be high as they are not wired
	jr nz, try_acia

	; Repeat the check on SIO B
	in a,(SIOB_C)
	and #0x2C
	cp #0x2C
	jr z, is_sio
try_acia:
	;
	;	Look for an ACIA
	;
	ld a,#ACIA_RESET
	out (ACIA_C),a
	; TX should now have gone
	in a,(ACIA_C)
	bit 1,a
	jr z, not_acia_either
	;	Set up the ACIA

        ld a, #ACIA_RTS_LOW_A
        out (ACIA_C),a         		; Initialise ACIA
	ld a,#2
	ld (_ser_type),a
	jp serial_up

	;
	; Doomed I say .... doomed, we're all doomed
	;
	; At least until RC2014 grows a nice keyboard/display card!
	;
not_acia_either:
	xor a
	ld (_ser_type),a
	jp serial_up
;
;	We have an SIO so do the required SIO hdance
;
is_sio:	ld a,b
	ld (_ser_type),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

serial_up:
        im 1 ; set CPU interrupt mode
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
; We use the MIDI port for debug - it's not useful for much else after all.
;
outchar:
	push af
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

_platform_doexec	.equ	0x18

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
	    ei
	    jp (hl)
	    nop
	    nop
rst20:	    ret
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
rst30:	    jp syscall_high
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
ldir_to_user:
	    ld de,#0x0181		; from bank 0 to bank  1
ldir_far:
	    push bc
	    ld c,#0x38
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
	    ld a,#1
	    out (0x38),a
	    ret
ldir_from_user:
	    ld de,#0x8101
	    jr ldir_far
;
;	High stubs. Present in each bank in the top 256 bytes
;
interrupt_high:
	    push af
	    push de
	    push hl
	    ld a,(banknum)
	    ld l,a
	    ld a,#0x01
	    out (0x38),a		; Bank 0, no ROM
	    ld (istack_switched_sp),sp	; istack is positioned to be valid
	    ld sp,#istack_top		; in both banks. We just have to
	    push hl			; save return bank
	    call interrupt_handler	; switch on the right SP
	    pop hl
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
	    ; User stack is now valid
	    ; back on user stack
	    xor a
	    cp e
	    call nz, sigpath
	    pop hl
	    pop de
	    pop af
	    ei
	    ret
kernout:
	    ; restore bank - if we interrupt mid user copy or similar we
	    ; have to put the right bank back
	    ld a,l
	    out (0x38),a
	    pop hl
	    pop de
	    pop af
	    ei
	    ret
	    
sigpath:
	    push de		; signal number
	    ld de,#irqsigret
	    push de		; clean up
	    jp (hl)
irqsigret:
	    inc sp		; drop signal number
	    inc sp
	    ret

syscall_high:
	    push ix
	    ld ix,#0
	    add ix,sp
	    push de		; the syscall if must preserve de for now
				; needs fixing when we change the syscall
				; API for Z80 to something less sucky
	    ld a,4(ix)
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
	    ex af, af'		; Ick - find a better way to do this bit !
	    push af
	    ld a,#1
	    out (0x38),a
	    pop af
	    ex af,af'
	    ; Stack now invalid
	    ld (U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    call unix_syscall_entry
	    ; FIXME check di rules
	    ; stack now invalid. Grab the new sp before we unbank the
	    ; memory holding it
	    ld sp,(U_DATA__U_SYSCALL_SP)
	    ld a,#0x81		; back to the user page
	    out (0x38),a
	    xor a
	    cp h
	    call nz, syscall_sigret
	    ; FIXME for now do the grungy C flag HL DE stuff from
	    ; lowlevel-z80 until we fix the ABI
	    ld bc,#0
	    ld a,h
	    or l
	    jr nz, error
	    ex de,hl
	    pop de
	    pop ix
	    ei
	    ret
error:	    scf
	    pop de
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
	    ret
syscall_sighelp:
	    pop de		; discard signal
	    pop de		; recover error info
	    pop hl
	    ld h,#0		; clear signal bit
	    ret

