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
	    .globl _platform_doexec
	    .globl _platform_reboot
	    .globl _platform_copier_l
	    .globl _platform_copier_h

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
	    .globl _keyin

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel.def"

KERNEL_HIGH	.equ	0
KERNEL_LOW	.equ	2		; 0/1 high 2/3 low
VIDEO_LOW	.equ	4

; -----------------------------------------------------------------------------
;
;	Because of the 32K split our memory model is a bit different to the
;	usual Z80 setup.
;
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_platform_monitor:
	    di
	    halt			; NMI button will cause a reboot..
_platform_reboot:
	    xor a
	    out (250), a		; ROM appears low
	    rst 0			; bang

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below common, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _DISCARD

; colours  - FIXME correct these
	    .db	0,32,64,120
clutmap:
init_early:
	    ld a, #0x44		; Kernel is in 0/1 2/3, so video goes above it
				; Video mode 3 (with luck)
	    out (252), a
	    ld a, #0x10		; black border, mic 1
	    out (254), a
	    in a, (251)
	    and #0x9F		; low palette colours
	    ld hl, #clutmap
	    ld bc, #4*256 + 248	; 4 colours, port 248
	    otdr
            ret

init_hardware:
            ; set system RAM size (FIXME - check for 512/1M)
            ld hl, #256
            ld (_ramsize), hl
            ld hl, #(256-64)		; 64K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode

	    ; interrupt mask
	    ; 60Hz timer on

            ret

	    .area _HIGH

_kernel_flag:
	    .db 1	; We start in kernel mode
low_map:
	    .db KERNEL_LOW
low_save:
	    .db 0
video_save:
	    .db 0

map_save_low:
	    push af
	    ld a,(low_map)
	    ld (low_save),a
	    pop af
	    ; Fall through as we also map the kernel
map_kernel_low:
	    push af
	    ld a,#KERNEL_LOW
	    ld (low_map),a
	    out (250),a
	    pop af
	    ret
map_restore_low:
	    ld a, (low_save)
	    jr map_page_low
map_user_low:
	    ld a,(U_DATA__U_PAGE)
map_page_low:
	    ld (low_map),a
	    out (250),a
	    ret
map_video:
	    ld a,(low_map)
	    ld (video_save),a
	    ld a,#VIDEO_LOW
	    jr map_page_low
unmap_video:
	    ld a,(video_save)
	    jr map_page_low

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

	    ; Copy the stub page into place
            ld hl, #stub_page
            ld de, #0x0000
            ld bc, #0x0100
            ldir
	    jp map_kernel_low

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
	    ld b, #0xFE
keyscl:	    ld c, #249			; need BC out to get address lines
	    in a, (c)
	    and #0xE0			; top 3 bits valid
	    ld d, a
	    ld c, #254
	    in a, (c)
	    and #0x1F			; low 5 bits
	    or d
	    ld (hl), a
	    inc hl
	    scf
	    rlc b
	    jr c, keyscl
	    in a, (c)
	    and #0x1F
	    ld (hl), a
	    ret

	    .area _PAGE0
;
;	Loaded into 00-FF in the bottom of the kernel and then into other
;	pages. There is an argument for putting most of this at the top of
;	the high page of everything. That might allow us to run CP/M
;	emulation
;
;	Move what we can into PAGEH and high kernel code
;
;	We'd still need some low space but we have 0A-3A/3B-4F/57-5B
;
;	We can't deal with NMI in the CP/M case though.
;
;	Starts at address 1 to avoid an SDCC flaw
;
stub_page:

stub0:	    .word 0		; cp/m emu changes this
	    .byte 0		; cp/m emu I/O byte
	    .byte 0		; cp/m emu drive and user
	    jp 0		; cp/m emu bdos entry point
rst8:
_platform_copier_l:
	    ld a,(hl)
	    out (251),a
	    exx
	    ldir
	    exx
	    nop			; fall through
rst10:	    ld a,#KERNEL_HIGH
	    out (251),a
	    ret
	    nop
	    nop
	    nop
rst18:
_platform_doexec:
	    out (251),a		; FIXME: trashing of clut high bits
	    ei
	    jp (hl)
	    nop
rst20:	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
	    nop
rst28:	    jp syscall_path
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
rst38:	    ; Interrupt handling stub
	    push af
	    push de
	    push hl
	    ; FIXME: this trashes any clut overrides
	    ld a,#KERNEL_HIGH
	    out (251),a
	    ; Kernel is now the high map. Our stack may be invalid
	    ; so be careful
	    ld (istack_switched_sp),sp
	    ld sp,#istack_top
	    call interrupt_handler
	    ; On return HL = signal vector E= signal (if any) A = page for
	    ; high
	    ; FIXME  0 = kernel do we need to xlate ?
	    out (251),a
	    ; stack is invalid again now
	    ld sp,(istack_switched_sp)
	    ; back on user stack
	    xor a
	    cp e
	    call nz, sigpath
	    pop hl
	    pop de
	    pop af
	    ei
	    ret
	    nop
	    nop
	    nop
nmi_handler:		; FIXME: check ends up at 0066
	    retn
sigpath:
	    push de		; signal number
	    ld de,#irqsigret
	    push de		; clean up
	    jp (hl)
irqsigret:
	    inc sp		; drop signal number
	    inc sp
	    ret

syscall_path:
	    push ix
	    ;TODO : BC/DE/HL/IX off stack
	    ; FIXME trashes the clut bits
	    ld a,#KERNEL_HIGH
	    di
	    out (251),a
	    ; Stack now invalid
	    ld (U_DATA__U_SYSCALL_SP),sp
	    ld sp,#kstack_top
	    call unix_syscall_entry
	    ; FIXME check di rules
	    ; On return A is the page (0 means kernel), 
	    ; FIXME: do we need to xlate
	    out (251),a
	    ; stack now invalid
	    ld sp,(U_DATA__U_SYSCALL_SP)
	    xor a
	    cp h
	    call nz, syscall_sigret
	    ; FIXME for now do the grungy C flag HL DE stuff from
	    ; lowlevel-z80 until we fix the ABI
	    ld bc,#0
	    pop ix
	    ret
syscall_sigret:
	    push hl		; save errno
	    push de		; save retval
	    ld l,h
	    ld h,#0
	    push hl		; signal
	    ld hl,#syscall_sighelp
	    push bc		; vector
	    ret
syscall_sighelp:
	    pop de		; discard signal
	    pop de		; recover error info
	    pop hl
	    ld h,#0		; clear signal bit
	    ret

	    .area _PAGEH
;
;	High stubs. Present in each bank
;
_platform_copier_h:
	    ld a,(hl)
	    out (251),a
	    exx
	    ldir
	    exx
	    ld a,#KERNEL_HIGH
	    out (251),a
	    ret
