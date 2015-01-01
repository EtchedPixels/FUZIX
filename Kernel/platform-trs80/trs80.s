;
;	    TRS 80  hardware support
;

            .module trs80

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
	    .globl platform_interrupt_all
	    .globl _kernel_flag

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

            ; exported debugging tools
            .globl _trap_monitor
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

init_hardware:
            ; set system RAM size
            ld hl, #128
            ld (_ramsize), hl
            ld hl, #(128-64)		; 64K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode

	    ; interrupt mask
	    ; 60Hz timer on

	    ld a, #0x24		; 0x20 for serial
	    out (0xe0), a
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

opsave:	    .db 0x06
_opreg:	    .db 0x06	; kernel map, 80 columns
_modout:    .db 0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz
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

;
;	Mapping set up for the TRS80 4/4P
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks. On a
;	4 or 4P without add in magic thats 0x62 and 0x63 mappings.
;
map_kernel:
	    push af
	    ld a, (_opreg)
	    and #0x8C		; keep video bits
	    or #0x02		; map 2, base memory
	    ld (_opreg), a
	    out (0x84), a
	    pop af
	    ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (_opreg)
	    and #0x8C
	    or (hl)		; udata page
	    ld (_opreg), a
	    out (0x84), a
            ret

map_process_a:			; used by bankfork
	    push af
	    push bc
	    ld b, a
	    ld a, (_opreg)
	    and #0x8C
	    or b
	    ld (_opreg), a
	    out (0x84), a
	    pop bc
	    pop af
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
	    ld a, (_opreg)
	    and #0x73
	    ld (opsave), a
	    pop af
	    ret

map_restore:
	    push af
	    push bc
	    ld a, (opsave)
	    ld b, a
	    ld a, (_opreg)
	    and #0x8C
	    or b
	    ld (_opreg), a
	    out (0x84), a
	    pop bc
	    pop af
	    ret
	    
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
