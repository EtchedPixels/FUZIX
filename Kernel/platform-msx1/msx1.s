;
;	    MSX1 hardware support
;

            .module msx1

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl _set_initial_map
	    .globl _need_resched
	    .globl _copy_vectors

	    ; video driver
	    .globl _vtinit

            ; exported debugging tools
            .globl _platform_monitor
            .globl outchar

            .globl unix_syscall_entry
            .globl _platform_reboot
	    .globl nmi_handler
	    .globl null_handler
	    .globl map_process_always
	    .globl map_kernel
	    .globl _vdp_load_font

	    .globl find_ram

	    ; We use buffer 0 data as a bounce buffer early in boot
	    .globl _bufpool

	     ; debug symbols
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

	    ; stuff to save
	    .globl _vdpport
	    .globl _vdptype
	    .globl _infobits
	    .globl _machine_type

	    .globl _int_disabled
	    ;
	    ; vdp - we must initialize this bit early for the vt
	    ;
	    .globl vdpinit

	    .globl s__DISCARD

            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

; This needs to live in common space on this platform

_int_disabled:
	   .db 1

; Ideally return to any debugger/monitor
_platform_monitor:
	    di
	    halt


_platform_reboot:
;FIXME: TODO
	    di
	    halt

_need_resched:
	   .db 0


; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, #'*'
	    out (0x2F), a
	    ld a, #'-'
	    call outchar
	    ; called with e'=vdp d'=machine type
	    ; HL info bits
	    exx
	    ld a,d
	    ld (_machine_type),a
	    ld a,c
	    inc a
	    ld (_vdpport),a
	    ld (_infobits),hl
            ret

init_hardware:
	    ld a, #'0'
	    out (0x2F), a

	    call _set_initial_map

	    ld a, #'1'
	    out (0x2F), a

	    call find_ram

	    ld a, #'2'
	    out (0x2F), a

	    ; Program the video engine

	    ld bc,(_vdpport)
	    ; Play with status register 2
	    dec c
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
	    in a,(c)
	    ld a,#2
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
vdpwait:    in a,(c)
	    and #0xE0
	    add a
	    jr nz, not9918a	; bit 5 or 6
	    jr nc, vdpwait	; not bit 7
	    ; we vblanked out - TMS9918A
	    xor a
	    ld (_vdptype),a
	    jr vdp_setup
not9918a:   ; Use the version register
	    ld a,#1
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
	    in a,(c)
	    rrca
	    and #0x1F
	    inc a
	    ld (_vdptype),a
vdp_setup:
	    call vdpinit
	    call _vdp_load_font

	    ld a, #'3'
	    out (0x2F), a

            im 1 			; set CPU interrupt mode
	    call _vtinit		; init the console video

	    ld a, #'4'
	    out (0x2F), a
	    ret

_program_vectors:
	    ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM
; emulator debug port for now
outchar:
	    push af
	    out (0x2F), a
	    pop af
            ret

;
;	Called early in boot to get the vectors and stubs into both banks
;	Use the data space of the first disk buffer as a bounce buffer
;
_copy_vectors:
	    ld hl,#0
	    ld de,#_bufpool
	    ld bc,#256
	    ldir
	    call map_process_always
	    dec h		; pointers back
	    dec d
	    inc b		; bc = 256
	    ex de,hl		; swap so we copy back
	    ldir		; into user
	    jp map_kernel
