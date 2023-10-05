;
;    ZX Evolution hardware support
;

        .module zxevo

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler
	.globl nmi_handler

	.globl map_buffers
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
        .globl _plt_monitor
	.globl _plt_reboot
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
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor as wait for space
	;	(or should we jump into the profi rom ?)
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _plt_monitor

_plt_reboot:
	di
	; FIXME: put back boot ROM and all the rest
        rst 0		; back into our booter

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0


; -----------------------------------------------------------------------------
; KERNEL CODE BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
        ret

	.area _DISCARD

init_hardware:
        ; set system RAM size
        ld hl, #4096
        ld (_ramsize), hl
        ld hl, #720	      	; FIXME: TBD	 (15 * 48K for now)
        ld (_procmem), hl

	; Set up video etc

	; EFE7 bit 0 0 bit 5 0
	; xx77		bit 3: 1 = 14Mhz
	; xx77 bits 2-0: 111  ATM video

	ld a,#0x01
	out (0xBF),a		; Shadow on

	ld bc,#0xFF77		; MMU on, hold off, res off
	ld a,#0x0F		; 14MHz, 80x25 text mode
	out (c),a

	xor a
	out (0xBF),a		; Shadow back off

        ; screen initialization
	call _vtinit

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
	push hl
	ld hl,(_udata + U_DATA__U_PAGE)
map_write_hl:
	ld (current_map),hl
	;
	;	The map is stored with page number complements
	;
	ld a,#0x01
	out (0xBF),a		; Shadow on
	ld bc,#0x37F7
	outi
	ld b,#0x77
	outi
	ld b,#0xB7
	outi
	xor a
	out (0xBF),a		; Shadow on
	pop hl
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
map_buffers:
map_kernel_restore:
	push af
	push bc
	push hl
	ld hl, #kernel_map
	jr map_write_hl

map_video:
	push af
	push bc
	push hl
	ld hl, #video_map
	jr map_write_hl

map_restore:
	push af
	push bc
	push hl
        ld hl, (map_store)
	jr map_write_hl

;
;	Debug output
;
outchar:
	push bc
outcw:
	ld c,#0xFDEF
	in a,(c)
	and #0x20
	jr z, outcw
	ld b,#0xF8
	out (c),a
	pop bc
        ret

	.area _COMMONDATA
_tmpout:
	.db 1

current_map:                ; place to store current page pointer
        .dw 0
map_store:		    ; and the saved one
        .dw 0

	; TODO FIXME
kernel_map:
	.db 0, 1, 2, 3
video_map:
	.db 0, 1, 8, 3
