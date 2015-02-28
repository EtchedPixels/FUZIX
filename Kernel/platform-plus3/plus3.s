;
;    ZX Spectrum Plus 2A and Plus 3 hardware support
;

        .module plus3

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
        .globl map_process
        .globl map_process_always
        .globl map_save
        .globl map_restore
	.globl map_process_save
	.globl map_kernel_restore
	.globl map_video
	.globl unmap_video

        .globl _kernel_flag
	.globl port_map

        ; exported debugging tools
        .globl _trap_monitor
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

        .include "kernel.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (above 0xC000 in page 3)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _trap_monitor

_trap_reboot:
	di
	im 1
	ld bc, #0x7ffd
	xor a		; 128K ROM, initial banks, low screen
	out (c), a
        rst 0		; Into the ROM

platform_interrupt_all:
        ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
	ld bc, #0x7ffd
	ld a, #3 + 8		  ; Screen into page 7
        ret

	.area _VIDEO

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #(128 - 64)        ; 64K for kernel/screen/etc
        ld (_procmem), hl

	call map_video
        ; screen initialization
        ; clear
        ld hl, #0xC000
        ld de, #0xC001
        ld bc, #0x1800            ; There should be 0x17FF, but we are going
        xor a                     ; to copy additional byte to avoid need of
        ld (hl), a                ; DE and HL increment before attribute
        ldir                      ; initialization (2 bytes of RAM economy)

        ; set color attributes
        ld a, #7   	         ; black paper, white ink
        ld bc, #0x300 - #1
        ld (hl), a
        ldir

	call unmap_video
        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	di
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

        ; set restart vector for UZI system calls
        ld (0x0030), a   ;  (rst 30h is unix function call vector)
        ld hl, #unix_syscall_entry
        ld (0x0031), hl

        ; Set vector for jump to NULL
        ld (0x0000), a   
        ld hl, #null_handler  ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl
	jr map_kernel

switch_kernel:
	ld a, (port_map)
	and #0xF8		; Preserve the other bits
	or #0x05		; Map 4,5,6,3
switchit:
	push bc
	ld (port_map), a
	ld bc, #0x1FFD
	out (c), a
	pop bc
	ret

switch_user:
	ld a, (port_map)
	and #0xF8		; Preserve the other bits
	and #0x01		; Map 0,1,2,3
	jr switch_kernel

switch_video:
	ld a, (port_map)
	or #0x7			; Map 4,7,6,3
	jr switchit

map_process:
        ld a, h
        or l
        jr z, map_kernel
map_process_save:
map_process_always:
	push af
	call switch_user
	pop af
	ret

unmap_video:
map_kernel:
map_kernel_restore:
	push af
	call switch_kernel
	pop af
	ret

map_video:
	push af
	call switch_video
	pop af
	ret

map_save:
	push af
        ld a, (port_map)
	and #7
        ld (map_store), a
	pop af
        ret

map_restore:
	push af
	push hl
	ld a, (port_map)
	and #0xF8
	ld hl, #map_store
	or (hl)
	call switchit
	pop hl
	pop af
	ret
;
;	We have no easy serial debug output instead just breakpoint this
;	address when debugging.
;
outchar:
        ret

	.area _COMMONDATA
_kernel_flag:
        .db 1
port_map:                   ; place to store current map register values
        .db 0               ; because we have no ability to read 1ffd port
                            ; to detect what page is mapped currently 
map_store:
        .db 0
ksave_map:
        .db 0
