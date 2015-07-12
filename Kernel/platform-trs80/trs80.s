;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl platform_interrupt_all

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

	    ; video
	    .globl _video_setpixel
	    .globl _video_op
	    .globl _video_attr

            ; exported debugging tools
            .globl _trap_monitor
            .globl _trap_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl trap_illegal
            .globl outcharhex
	    .globl fd_nmi_handler
	    .globl null_handler
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl _opreg
	    .globl _modout

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	    di
	    halt

platform_interrupt_all:
	    in a,(0xef)
	    ret

_trap_reboot:
	   di
	   halt

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

_ctc6845:				; registers in reverse order
	    .db 99, 80, 85, 10, 25, 4, 24, 24, 0, 9, 101, 9, 0, 0, 0, 0
init_early:
	    ld a, (_opreg)
	    out (0x84), a
	    ld a, (_modout)
	    out (0xEC), a

            ; load the 6845 parameters
	    ld hl, #_ctc6845
	    ld bc, #0x1088
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x89), a		; data
	    inc hl
	    dec b
	    jp p, ctcloop

   	    ; clear screen
	    ld hl, #0xF800
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0xF802
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
            ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
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

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #fd_nmi_handler
            ld (0x0067), hl
	    jp map_kernel
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            out (0xEB), a
            ret

;
;	Swap helpers
;
_hd_xfer_in:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   call map_kernel
	   ret

_hd_xfer_out:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
	   call map_kernel
	   ret

;
;	Graphics card
;
setpixel_optab:
	    nop				;
	    or b			; COPY
	    nop
	    or b			; SET
	    cpl				; complement pixel mask
	    and b			; CLEAR by anding with mask
	    nop
	    xor b			; INVERT
setpixel_bittab:
	    .db 128,64,32,16,8,4,2,1

_video_setpixel:
	    ld a, (_video_attr + 2)	; mode
	    or a			; copy ?
	    jr nz, setpixel_notdraw
	    ld a, (_video_attr)		; ink
	    or a			; white ?
	    jr nz, setpixel_notdraw	; a = 1 = set so good
	    ld a, #2			; clear
setpixel_notdraw:
	    ld e, a
	    ld d, #0
	    ld hl, #setpixel_optab
	    add hl, de
	    ld a, (hl)
	    ld (setpixel_opcode), a	; Self modifying
	    inc hl
	    ld a, (hl)
	    ld (setpixel_opcode), a	; Self modifying
	    ld bc, (_video_op)	; B is the count
	    ld a, b
	    and #0x1f		; max 31 pixels per op
	    ret z
	    push bc
setpixel_loop:
	    ld hl, #_video_op + 2	; co-ordinate pairs
	    ld a, (hl)		; low bits of X
	    and #7
	    ld c, a
	    ld a, (hl)
	    inc hl
	    ld b, (hl)		; high bits of X
	    srl b
	    rra
	    srl b
	    rra
	    srl b
	    rra
	    and #0x7F
	    out (0x80), a
	    ld a, (hl)		; y low (no y high needed)
	    inc hl
	    inc hl		; next point pair
	    push hl
	    out (0x81), a
	    ld hl, #setpixel_bittab
	    ld b, #0
	    add hl, bc
	    ld b, (hl)		; our pixel mask
	    in a, (0x82)	; pixel from screen
setpixel_opcode:
	    nop			; nop or cpl
	    or b
	    out (0x82), a
	    pop hl
	    pop bc
	    djnz setpixel_loop
	    ret
