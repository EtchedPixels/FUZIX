;
;	    SAM Coupe initial hardware support
;

            .module sam

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl _kernel_flag
	    .globl map_page_low
	    .globl map_kernel_low
	    .globl map_user_low
	    .globl map_save_low
	    .globl map_restore_low
	    .globl map_video
	    .globl unmap_video
	    .globl _plt_doexec
	    .globl _plt_reboot
	    .globl _plt_copier_l
	    .globl _plt_copier_h
	    .globl _keyscan
	    .globl _mousescan
	    .globl _mouse12
	    .globl _mouse_probe
	    .globl _int_disabled
	    .globl _udata
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
	    .globl _keyin
	    .globl _mousein

	    .globl _vtwipe
	    .globl _vtinit

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

KERNEL_LOW	.equ	0x20		; 0/1 low 2/3 high (ROM off)
KERNEL_HIGH	.equ	2
VIDEO_LOW	.equ	4		; 4/5 video and font

; -----------------------------------------------------------------------------
;
;	Because of the 32K split our memory model is a bit different to the
;	usual Z80 setup.
;
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_plt_monitor:
	    di
	    halt			; NMI button will cause a reboot..
_plt_reboot:
	    xor a
	    out (250), a		; ROM appears low
	    rst 0			; bang

_int_disabled:
	    .db 1

; -----------------------------------------------------------------------------
;	We have discard mapped high up so we can map stuff under it
;	during setup. All of discard gets reclaimed when init is run
; -----------------------------------------------------------------------------
            .area _DISCARD

	    ; Mode 3 colour set we use
	    .byte 0x00		; Black
	    .byte 0x40		; Green
	    .byte 0x20		; Red
clutmap:
	    .byte 0x70		; White

init_early:
	    ld a, #0x44		; Kernel is in 0/1 2/3, so video goes above it
				; Video mode 3 (with luck)
	    out (252), a
	    ld a, #0x10		; black border, mic 1
	    out (254), a
	    ld hl, #clutmap	; set low palette colours
	    ld bc, #4*256 + 248	; 4 colours, port 248
	    otdr
            ret

init_hardware:
            ; set system RAM size (FIXME - check for 512/1M)
	    ld a,#0x10			; Upper 256K
	    call map_page_low
	    ld hl,#0x0000
	    ld (hl),#0xff
	    ld a,(hl)
	    inc a
	    jr nz, is256k
	    ld (hl),a
	    ld a,(hl)
	    or a
	    ld hl,#512
	    jr z, is512k
is256k:	    
            ld hl, #256
is512k:
            ld (_ramsize), hl
	    ld de,#96			; 64K for kernel, 32K for video etc
	    sbc hl,de			; C is always clear here
            ld (_procmem), hl

	    ld a,#VIDEO_LOW
	    call map_page_low		; Map the video in the low 32K

	    ; Place a copy of the high stubs into the video bank so that we
	    ; can in future field interrupts with video mapped. Quite a bit
	    ; of other work is needed for this due to the stack situation.
	    ld hl,#0xFF00
	    ld de,#0x7F00
	    ld bc,#0x100
	    ldir

	    call map_kernel_low

	    ; Make a copy of the low page somewhere accessible. This may
	    ; change if we decide to make the kernel low page different - eg
	    ; to fast path interrupts in kernel mode. FIXME
	    ld hl,#0x0000
	    ld de,#lowstubs
	    ld bc,#0x0068
	    ldir

            im 1 ; set CPU interrupt mode

	    ; interrupt mask
	    ; 50Hz timer on

	    call _vtwipe
	    call _vtinit

            ret

	    .area _HIGH

_kernel_flag:
	    .db 1	; We start in kernel mode
low_save:
	    .db 0
video_save:
	    .db 0
entry_low:
	    .db 0	; Holds the low bank value on IRQ entry, Used so
			; we can save and restore temporary kernel mappings

map_save_low:
	    push af
	    in a,(250)
	    ld (low_save),a
	    pop af
	    ; Fall through as we also map the kernel
map_kernel_low:
	    push af
	    ld a,#KERNEL_LOW
	    out (250),a
	    pop af
	    ret
map_restore_low:
	    ld a, (low_save)
	    jr map_page_low
map_user_low:
	    ld a,(_udata + U_DATA__U_PAGE)
map_page_low:
	    or #0x20			; force ROM off
	    out (250),a
	    ret
;
;	The video map requires interrupt disables. Do the disable in
;	map_video. Save the old state in af' and then restore int in
;	unmap_video
;
;
map_video:
	    ld a,(_int_disabled)
	    ex af,af'
	    di
	    in a,(250)
	    ld (video_save),a
	    ld a,#VIDEO_LOW
	    jr map_page_low
unmap_video:
	    ld a,(video_save)
	    call map_page_low
	    ex af,af'
	    or a
	    ret nz
	    ei
	    ret

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    ld a,(hl)	; get the low page
	    call map_page_low

	    exx
	    ; Copy the stub page into place
            ld hl, #lowstubs
            ld de, #0x0000
            ld bc, #0x0068
            ldir
	    exx

	    ; And then the high stubs from the kernel
	    inc hl
	    ld a,(hl)
	    call map_page_low
	    ld hl,#stubs_high
	    ld de,#stubs_high-0x8000
	    ld bc,#0x0100
	    ldir

	    jp map_kernel_low
;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
; We use the MIDI port for debug - it's not useful for much else after all.
;
outchar:
	    push af
outcw:	    in a,(248)
	    bit 1,a
	    jr nz, outcw
	    pop af
	    out (253),a
            ret
;
;	Keyboard scanner. This is easiest kept in asm space
;	(reference code from the technical manual)
;
_keyscan:
	    ld hl, #_keyin
	    ld bc, #0xFEFE		; F9 is keyboard port (254)
keyscl:
	    ; Read low 5 bits from port F9, with the mask in the top address
	    ; lines (contents of B)
	    in e, (c)
	    ; Do the same for port FE for the upper lines, load A before
	    ; so we keep the mask right
	    ld a,b
	    in a, (0xF9)
	    xor e
	    and #0xE0			; keep top 3 bits
	    xor e
	    ; Save our decode
	    cpl
	    ld (hl), a
	    inc hl
	    rlc b
	    jr c, keyscl
	    ; Have we done all the lines (FE FD FB F7 EF DF BF 7F)
	    ; Now do FF which is the special case as we have no high bits
	    ; attached to it.
	    ld b,#0xFF
	    in a, (c)
	    cpl
	    and #0x1F
	    ld (hl), a
	    ret

_mousescan:
	    ld hl,#0
	    ld bc,#0xfffe
	    in a,(c)
	    in a,(c)		; should be xxxx1111
	    or #0xf0		; should now be 11111111
	    inc a		; cheap way to and 15, cp 15...
	    ret nz		; was not right - punt return 0
	    ld hl,#_mousein
	    ini			; fetch and store buttons
	    inc b		; fix up B (needs to be FFFE for the port)
	    ini			; y256
	    inc b
	    ini			; y16
	    inc b
	    ini			; y1
	    inc b
	    ini			; x256
	    inc b
	    ini			; x16
	    inc b
	    ini			; x1
	    inc b
	    in a,(c)		; end
	    ret			; hl will be non zero

_mouse12:
	    ; Get the top nibble
	    ld a,(hl)
	    and #0x0f
	    ; Sign extend it if needed
	    bit 3,a
	    jr z, nosex
	    or #0xf0
nosex:
	    ld d,a
	    ; Next nibble is bits 7-4
	    inc hl
	    ld a,(hl)
	    and #0x0f
	    rlca
	    rlca
	    rlca
	    rlca
	    ld e,a
	    ; and bits 3-0
	    inc hl
	    ld a,(hl)
	    and #0x0f
	    ; Merge to get low byte
	    or e
	    ld l,a
	    ; Get high byte
	    ld h,d
	    ret

	    .area _DISCARD

_mouse_probe:
	    ld bc,#0xfffe
	    ld hl,#0x0b01	; 0b for count, 01 so can ret nz
	    in a,(c)
	    in a,(c)
	    or #0xf0
	    inc a
	    ret nz
probe_in:
	    in a,(c)
	    dec h
	    jr nz, probe_in
	    and #0x0f
	    ld l,a		; 0 if present
	    ret

	    .area _CODE
;
;	Real time clock on port 239
;
;	The upper address bits select the actual RTC port read or
;	written.
;
;	This appears to be an OKI6242B
;

	    .globl _samrtc_in
	    .globl _samrtc_out

_samrtc_in:
	    ld b,l
	    ld c,#239
	    in a,(c)
	    and #0x0f
	    ld l,a
	    ret
_samrtc_out:
	    ld b,l
	    ld c,#239
	    out (c),h
	    ret


	    .area _PAGE0
;
;	This exists at the bottom of each process mapping we are using
;
;	Starts at address 1 to avoid an SDCC flaw
;
stub_page_1:

stub0:	    .word 0		; cp/m emu changes this
	    .byte 0		; cp/m emu I/O byte
	    .byte 0		; cp/m emu drive and user
	    jp 0		; cp/m emu bdos entry point
rst8:
_plt_copier_l:		; Must be low
	    ld a,(hl)
	    out (251),a
	    exx
	    ldir
	    exx
	    and #0x60		; preserve the colour bits
	    or #KERNEL_HIGH
	    out (251),a
	    jp (ix)
syscall_stash:
	    .byte 0		; must be low
rst18:
_plt_doexec:
	    out (251),a		; caller needs to handle CLUT bits
	    ei
	    jp (hl)
	    nop
	    nop
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

	    .area _PAGEH

stubs_high:
;
;	High stubs. Present in each bank in the top 256 bytes
;
_plt_copier_h:
	    ld a,(hl)
	    out (251),a
	    exx
	    ldir
	    exx
	    and #0x60
	    or #KERNEL_HIGH
	    out (251),a
	    jp (ix)
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

	    in a,(251)
	    and #0x60
	    or #KERNEL_HIGH
	    out (251),a
	    ; Kernel is now the high map. Our stack may be invalid
	    ; so be careful
	    ld (istack_switched_sp),sp
	    ld sp,#istack_top
	    call interrupt_handler
	    ; Restore stack pointer to user. This leaves us with an invalid
	    ; stack pointer if called from user but interrupts are off anyway
	    ld sp,(istack_switched_sp)
	    ; On return HL = signal vector E= signal (if any) A = page for
	    ; high
	    or a
	    jr z, kernout
	    ; Returning to user space
	    ld d,a
	    in a,(251)
	    and #0x60
	    or d
	    out (251),a
	    ; User stack is now valid
	    ; back on user stack
	    xor a
	    cp e
	    call nz, sigpath
kernout:
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
	    push iy
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
	    ld (syscall_stash),a
	    in a,(251)
	    and #0x60
	    or #KERNEL_HIGH
	    out (251),a
	    ; Stack now invalid
	    ld (_udata + U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    ld a,(syscall_stash)
	    call unix_syscall_entry
	    ; FIXME check di rules
	    pop iy
	    push bc
	    ld b,a
	    in a,(251)
	    and #0x60
	    or b
	    pop bc
	    ; stack now invalid. Grab the new sp before we unbank the
	    ; memory holding it
	    ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	    out (251),a
	    xor a
	    cp h
	    call nz, syscall_sigret
	    ; Decide what to return
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
	    push hl		; save errno
	    push de		; save retval
	    ld l,h
	    ld h,#0
	    push hl		; signal
	    ld hl,#syscall_sighelp
	    push hl		; vector return
	    push bc		; actual signal vector
	    ret			; to handler which will return to sighelp
syscall_sighelp:
	    pop de		; discard signal
	    pop de		; recover error info
	    pop hl
	    ld h,#0		; clear signal bit
	    ret

	    .area _DATA
lowstubs:
	    .ds 0x68
