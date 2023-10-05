;
;	Simple80 Support
;

        .module simple80

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
	.globl _ctc_present
	.globl outstring

	.globl _sys_stubs

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"


; Base address of SIO/2 chip 0x80

SIOA_D		.EQU	0x00
SIOA_C		.EQU	0x01
SIOB_D		.EQU	0x02
SIOB_C		.EQU	0x03

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
_plt_reboot:
	;
	; Call the helper 
	;
	di
	jp 0xFFE9		; kaboom 

_int_disabled:
	.db 1

; -----------------------------------------------------------------------------
;
;	Our MMU is write only, but if we put a value in each bank in a fixed
;	place we have a fast way to see which bank is which
;
; -----------------------------------------------------------------------------

banknum:
	.byte 0x58		; copied into far bank then set to 18

; -----------------------------------------------------------------------------
;	All of discard gets reclaimed when init is run
; -----------------------------------------------------------------------------
	.area _DISCARD

init_hardware:
	ld hl, #128
        ld (_ramsize), hl
	ld hl,#60		; We lose 4K to common space
        ld (_procmem), hl

	ld hl,#s__COMMONMEM
	ld de,#l__COMMONMEM

	; Modified board drives the RAM A16 differently
	ld a,(0xfffe)
	and #2
	call nz, modified_mem

	; This is run once so can be slow and simple
farput:
	ld a,(hl)
	push de
	ld de,#0xe8ea		; bank 1 and 0 if extended, ignored
				; if not
	call 0xFFE0		; put byte in a into (HL) in far memory
				; uses BC/DE/HL/AF
	pop de
	inc hl
	dec de
	ld a,d
	or e
	jr nz,farput

	ld a,(bits_from_user+1)
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

	ld hl,#sioa_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#siob_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

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
	cp #0xA9			; Should be one lower
	jr nz, no_ctc

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

BOOT_MAP .EQU	0xEA		; Bank map default

sioa_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x58
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte BOOT_MAP
siob_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x18
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte BOOT_MAP


	    .area _COMMONMEM

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

	.area _CODE
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
	    ld bc,(sio_reg)
	    out (c),b
	    ld a,(bits_to_user+1)
rst20:	    out (c),a
	    ei
	    jp (hl)
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
	    .globl modified_mem

	    .globl bits_from_user
	    .globl bits_to_user
	    .globl sio_reg
;
;	This needs some properly optimized versions!
;
;
bits_to_user:
	.word	0x5818
bits_from_user:
	.word	0x1858
sio_reg:
	.word	0x0103			; SIOB R 1


;
;	Configure the ldir helpers for a modified board
;
modified_mem:
	    ld hl,#0xe8ea
	    ld (bits_to_user),hl
	    ld a,h
	    ld (banknum),a
	    ld hl,#0xeae8
	    ld (bits_from_user),hl
            ld hl,#0x0503		; SIOB R 5
	    ld (sio_reg),hl
	    ld a,#0xea
	    ld a,l			; get the register
	    ld (bankpatch1+1),a
	    ld (bankpatch2+1),a
	    ld (bankpatch3+1),a
	    ld (bankpatch4+1),a
	    ret

;
;	Copy memory between banks
;
ldir_to_user:
	    ld de,(bits_to_user)		; from bank 0 to bank  1
ldir_far:
	    push bc
	    ld bc,(sio_reg)
	    exx
	    pop bc			; get BC into alt bank
	    di				; annoyingly : FIXME
far_ldir_1:
	    exx
	    out (c),b
	    out (c),e
	    ld a,(hl)
	    inc hl
	    out (c),b
	    out (c),d
	    ld (ix),a
	    inc ix
	    exx
	    dec bc
	    ld a,b
	    or c
	    jr nz, far_ldir_1
	    exx
	    out (c),b
	    ld de,(bits_to_user)
	    out (c),e
	    exx
	    ret
ldir_from_user:
	    ld de,(bits_from_user)
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
	    ld e,a
	    ;
	    ;	Switch to kernel
	    ;
	    ld bc,(sio_reg)
	    out (c),b
	    ld a,(bits_from_user+1)
	    out (c),a
	    ld (istack_switched_sp),sp	; istack is positioned to be valid
	    ld sp,#istack_top		; in both banks. We just have to
	    ;
	    ;	interrupt_handler may come back on a different stack in
	    ;	which case bc is junk. Fortuntely we never pre-empt in
	    ;	kernel so the case we care about bc is always safe. This is
	    ;	not a good way to write code and should be fixed! FIXME
	    ;
	    push de
	    call interrupt_handler	; switch on the right SP
	    pop bc	; C is now the return bank info
	    ld d,c	; as we need BC clear and D is free
	    ; Restore stack pointer to user. This leaves us with an invalid
	    ; stack pointer if called from user but interrupts are off anyway
	    ld sp,(istack_switched_sp)
	    ; On return HL = signal vector E= signal (if any) A = page for
	    ; high
	    or a
	    jr z, kernout
	    ;
	    ; Returning to user space
	    ;
	    ld bc,(sio_reg)		; User bank
	    out (c),b
	    ld a,(bits_to_user+1)
	    out (c),a
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
	    ; have to put the right bank back. BC is free at this point
	    ld a,c
	    ld bc,(sio_reg)
	    out (c),b
	    out (c),a
	    jr pops
	    
sigpath:
	    push de		; signal number
	    ld de,#irqsigret
	    push de		; clean up
	    ex de,hl
	    ld hl,(PROGLOAD+16)	; helper vector
	    jp (hl)
irqsigret:
	    inc sp		; drop signal number
	    inc sp
	    ret

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
	    ex af, af'		; Ick - find a better way to do this bit !
	    ; FIXME: will need patching on the out calls to make this work
	    ; on the revised board
	    ld a,(sio_reg+1)
bankpatch1:
	    out (0x03),a
	    ld a,(bits_from_user+1)
bankpatch2:
	    out (0x03),a
	    ex af,af'
	    ; Stack now invalid
	    ld (_udata + U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    call unix_syscall_entry
	    ; stack now invalid. Grab the new sp before we unbank the
	    ; memory holding it
	    ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	    ;
	    ; FIXME: also needs patch for revised board
	    ;
	    ld a,(sio_reg+1)	; back to the user page
bankpatch3:
	    out (0x03),a
	    ld a,(bits_to_user+1)
bankpatch4:
	    out (0x03),a
	    xor a
	    cp h
	    call nz, syscall_sigret
	    pop bc
	    ld a,h
	    or l
	    jr nz, error
	    ex de,hl
	    pop ix
	    ei
	    ret
error:	    scf
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

