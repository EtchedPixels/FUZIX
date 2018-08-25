;
;	    SBC v2 support
;

            .module sbcv2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl map_for_swap
	    .globl platform_interrupt_all
	    .globl _kernel_flag

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl trap_illegal
            .globl outcharhex
	    .globl null_handler

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
;
; COMMON MEMORY BANK (kept even when we task switch)
;
            .area _COMMONMEM

platform_interrupt_all:
	    ret

; FIXME: map ROM and jump into it
_platform_monitor:
_platform_reboot:
	    di
	    xor a
	    out (0x7C),a
	    rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	    .area _CODE

;
;	We will address this one better when we do some ROMWBW integration
;
	    .globl _ttymap
init_early:
	    ; FIXME: code goes here to check for PropIO v2 nicely
	    ; Passes the minimal checking
	    ld hl,#0x0102
	    ld (_ttymap+1), hl		; set tty map to 0,2,1 for prop
	    ret

	    .area _DISCARD

init_hardware:
	    ld hl,#512
            ld (_ramsize), hl
	    ld hl,#448
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode

            ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

mapreg:    .db 0
mapsave:   .db 0

_kernel_flag:
	    .db 1	; We start in kernel mode

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_process

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

;
;	Mapping set up for the SBCv2
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
;
;	We know the ROM mapping is already off
;
map_kernel:
	    push af
	    ; ROMWBW TPA is last but one bank (last bank is high space)
	    ; so for now we hardcode this. We should ask ROMWBW at boot
	    ld a,#14
	    ld (mapreg),a
	    out (0x78), a
	    pop af
	    ret
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (hl)
map_for_swap:
map_process_a:			; used by bankfork
	    dec a		; We bias by 1 because 0 is a valid user
	    ld (mapreg), a	; bank
	    out (0x78), a
	    inc a		; cheaper than push/pop
            ret

map_process_always:
	    push af
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

map_save:   push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (mapsave)
	    ld (mapreg), a
	    out (0x78), a
	    pop af
	    ret
	    
outchar:
	    push af
twait:	    in a,(0x6D)
	    bit 5,a
	    jr z, twait
	    pop af
	    out (0x68),a
            ret

;
;	PropIO v2 block transfers
;

; FIXME: Move these somewhere better
BLKPARAM_ADDR_OFFSET		.equ	0
BLKPARAM_IS_USER_OFFSET		.equ	2
BLKPARAM_SWAP_PAGE		.equ	3

	    .globl _platform_prop_sd_read
	    .globl _platform_prop_sd_write

	    .globl _blk_op

_platform_prop_sd_read:
	    ld a,(_blk_op + BLKPARAM_IS_USER_OFFSET)
	    ld hl, (_blk_op + BLKPARAM_ADDR_OFFSET)
	    or a
	    jr z, do_read
	    dec a
	    ld a, (_blk_op + BLKPARAM_SWAP_PAGE)
	    jr nz, do_read_a
	    ld a, (U_DATA__U_PAGE)
do_read_a:  call map_process_a
do_read:   ld bc,#0xAB
	    inir
	    inir
	    jp map_kernel

_platform_prop_sd_write:
	    ld a,(_blk_op + BLKPARAM_IS_USER_OFFSET)
	    ld hl, (_blk_op + BLKPARAM_ADDR_OFFSET)
	    or a
	    jr z, do_write
	    dec a
	    ld a, (_blk_op + BLKPARAM_SWAP_PAGE)
	    jr nz, do_write_a
	    ld a, (U_DATA__U_PAGE)
do_write_a: call map_process_a
do_write:   ld bc,#0xAB
	    otir
	    otir
	    jp map_kernel
