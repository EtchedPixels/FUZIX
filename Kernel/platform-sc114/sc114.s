;
;	SC114 Initial Support
;

        .module sc114

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
	.globl _scm_romcall

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

	.globl _scm_reset
	.globl _scm_conout
	.globl scm_farput

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

_int_disabled:
	.db 1

;
;	Ugly but the API happens to use A B and C so all the nice ways to
;	ROM page are not available. On the SC108 this will stick us back
;	in the kernel mapping but that's fine - we were there anyway
;
;	Interrupts must be off for a ROM call
;
_scm_romcall:
	di
	ex af,af'
	xor a
	out (0x38),a		; ROM in
	ex af,af'
	rst 0x30
	ex af,af'
	inc a
	out (0x38),a
	ex af,af'
	ld a,(_int_disabled)
	or a
	ret nz
	ei
	ret
_platform_monitor:
	; Reboot ends up back in the monitor
_platform_reboot:
	jp _scm_reset

	
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
	; A holds the platform
	cp #0x08		; SC114
	jr z, is_sc114
	; Nope - must be an SC108
	; Reprogram the mapping logic
	ld de,#0x0181
	ld (_bank0to1),de
	ld de,#0x8101
	ld (_bank1to0),de
	ld a,#0x38
	ld (_bankport),a
is_sc114:
	ld hl, #128
        ld (_ramsize), hl
	ld hl,#64
        ld (_procmem), hl

	ld de,#s__COMMONMEM
	ld bc,#l__COMMONMEM

	;
	;	Use the ROM helper to get our common into the secondary
	;	memory bank.
	;
install_common:
	ld a,(de)
	push bc
	push de
	call scm_farput
	pop de
	pop bc
	inc de
	dec bc
	ld a,b
	or c
	jr nz, install_common
	; A = 0
	; SC108 is already back in the right RAM bank, make sure SC114 is
	out (0x30),a
	ld a,#0x01
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

        im 1 ; set CPU interrupt mode
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
	push bc
	push de
	push hl
	call _scm_conout
	pop hl
	pop de
	pop bc
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
rst18:				; execve entry to user space
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

; FIXME: save a byte and some setup by makign byte 2 of 0to1 the first
; of 1to0
_bank0to1:
	   .dw 0x0001			; 0x0181 for SC108
_bank1to0:
	   .dw 0x0100			; 0x8101 for SC108
_bankport:
	   .db 0x30			; 0x38 for SC108
;
;	This needs some properly optimized versions!
;
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
 	    ld a,(_bank0to1)		; gets us the bank 0 value */
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
	    out (0x38),a		; Bank 0
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
	    ; BUG: syscall corrupts AF' - should we just define some
	    ; alt register corruptors for new API - would be sanest fix
	    ex af, af'		; Ick - find a better way to do this bit !
	    ld a,#0x01
	    out (0x38),a
	    rlca
	    out (0x30),a
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
	    rlca
	    out (0x30),a
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

