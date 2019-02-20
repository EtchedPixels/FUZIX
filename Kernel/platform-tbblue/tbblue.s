;
;    TBBlue hardware support
;

        .module tbblue

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl platform_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler
	.globl nmi_handler

        .globl map_kernel
        .globl map_process_always
        .globl map_process
        .globl map_kernel_di
        .globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_kernel_restore
	.globl map_video
	.globl current_map

	.globl _int_disabled
	.globl _vtborder

        ; exported debugging tools
        .globl _platform_monitor
	.globl _platform_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

	.globl _vtoutput
	.globl _vtinit

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (above 0xE000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_platform_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _platform_monitor

_platform_reboot:
	di
	; FIXME: put back 48K ROM
        rst 0		; back into our booter

platform_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0


; -----------------------------------------------------------------------------
; KERNEL CODE BANK (below 0xE000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
        ret

	.area _DISCARD

init_hardware:
        ; set system RAM size
        ld hl, #512		; FIXME: probe not hardcode
        ld (_ramsize), hl
        ld hl, #448	      	; FIXME: TBD
        ld (_procmem), hl

	; Reprogram the TBBlue registers

	ld a,#0x06
	ld de,#0x9B80	; Turbo on, DivMMC paging off, Multiface off, 
			; Sound mode AY
	call tbblue_config

	ld a,#0x07
	ld de,#0x0302	; 14Mhz
	call tbblue_config

	ld a,#0x08
	ld de,#0xEECE	; 128K paging(?), no RAM contention, ABC stereo
			; Covox, Timex, Turbosound on
	call tbblue_config

	ld a,#0x09
	ld de,#0xFC00	; Kempston on, divMMC on
	call tbblue_config

	ld a,#0x15
	ld de,#0xFF00	; Lores off, sprites off, border off, normal order
	call tbblue_config

	; Activate Timex screen
	ld a,#0x3E
	out (0xFF),a

        ; screen initialization
	call _vtinit

        ret

tbblue_config:
	ld bc,#0x243B
	out (c),a
	inc b
	ld a,d		; mask
	cpl		; bits to keep
	in d,(c)	; read register
	and d		; mask out bits to change
	or e		; mask in new bits
	out (c),a	; write register
	ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	pop de
	pop hl
	push hl
	push de
	call map_process

        ; write zeroes across all vectors
        ld hl, #0
        ld de, #1
        ld bc, #0x007f ; program first 0x80 bytes only
        ld (hl), #0x00
        ldir

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

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

	jr map_kernel

map_process:
        ld a, h
        or l
        jr z, map_kernel
map_process_always:
map_process_always_di:
	push af
	push bc
	push de
	push hl
	ld hl,(_udata + U_DATA__U_PAGE)
map_write_hl:
	; Switch this to nextreg at some point FIXME
	ld bc,#0x243b
	ld (current_map),hl
	ld e,#0x0750
map_write_loop:
	ld a,(hl)
	out (c),e
	inc b
	out (c),a
	dec b
	dec d
	jr nz, map_write_loop
	pop hl
	pop de
	pop bc
	pop af
	ret

;
;	Save and switch to kernel
;
map_save_kernel:
	push hl
        ld hl, (current_map)
        ld (map_store), hl
	pop hl
map_kernel_di:
map_kernel:
map_kernel_restore:
	push af
	push bc
	push de
	push hl
	ld hl, #kernel_map
	jr map_write_hl

map_video:
	push af
	push bc
	push de
	push hl
	ld hl, #video_map
	jr map_write_hl

map_restore:
	push af
	push bc
	push de
	push hl
        ld hl, (map_store)
	jr map_write_hl

;
;	We have no easy serial debug output instead just breakpoint this
;	address when debugging.
;
outchar:
	ld (_tmpout), a
	push bc
	push de
	push hl
	push ix
	ld hl, #1
	push hl
	ld hl, #_tmpout
	push hl
	call _vtoutput
	pop af
	pop af
	pop ix
	pop hl
	pop de
	pop bc
        ret

	.area _COMMONDATA
_tmpout:
	.db 1

current_map:                ; place to store current page pointer
        .dw 0
map_store:		    ; and the saved one
        .dw 0

kernel_map:
	.db 16, 17, 18, 19, 20, 21, 22
video_map:
	.db 16, 17, 10, 11, 20, 21, 22
