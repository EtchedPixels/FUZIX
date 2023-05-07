;
;	    Banking logic for the EG64 board
;

        .module trs80bank

        ; exported symbols
        .globl init_hardware
	.globl map_kernel
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_buffers
	.globl map_process
	.globl map_process_di
	.globl map_process_always
	.globl map_process_always_di
	.globl map_for_swap
	.globl map_save_kernel
	.globl map_restore

        ; imported symbols
        .globl _ramsize
        .globl _procmem

	.globl interrupt_handler
	.globl nmi_handler
	.globl unix_syscall_entry
	.globl null_handler

	.globl _vt_check_lower

        .include "kernel.def"
        .include "../kernel-z80.def"

;
;	These live below 0x8000 so they are not switched out
;
        .area _COMMONMEM

init_hardware:
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
	
	ld hl,#96
	ld (_ramsize),hl
	ld hl,#39
	ld (_procmem),hl
        im 1 ; set CPU interrupt mode
	jp _vt_check_lower
;
;	Mapping for us is fairly simple but it's not blank because we do
;	some mapping.
;
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push af
	ld a,#0xC0		; Internal memory, ROM unmapped, IO
	ld (map_state),a
	out (0xC0),a
	pop af
	ret
map_process:
map_process_di:
map_for_swap:
map_process_always:
map_process_always_di:
	push af
	ld a,#0xD0		; External high, ROM unmapped, IO
	ld (map_state),a	; (so we can do screen mapping)
	out (0xC0),a
	pop af
	ret
map_save_kernel:
	push af
	ld a,(map_state)
	ld (map_save_val),a
	ld a,#0xC0		; Internal memory, ROM unmapped, IO
	ld (map_state),a
	out (0xC0),a
	pop af
	ret	
map_restore:
	push af
	ld a,(map_save_val)
	ld (map_state),a
	out (0xC0),a
	pop af
	ret

map_state:
	.db 0
map_save_val:
	.db 0
