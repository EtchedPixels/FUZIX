;
;	Z80Pack hardware support
;
;
;	This goes straight after udata for common. Because of that the first
;	256 bytes get swapped to and from disk with the uarea (512 byte disk
;	blocks). This isn't a problem but don't put any variables in here.
;
;	If you make this module any shorter, check what follows next
;


            .module px4plus

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl _kernel_flag

	    .globl platform_interrupt_all

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
	    .globl _vtinit

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    ; RAM drive
	    .globl _hd_xferin
	    .globl _hd_xferout
	    .globl _ramd_off
	    .globl _ramd_size
	    .globl _ramd_uaddr


	    .globl __bank_0_1
	    .globl __bank_0_2
	    .globl __bank_0_3
	    .globl __bank_1_2
	    .globl __bank_1_3
	    .globl __bank_2_1
	    .globl __bank_2_3
	    .globl __bank_3_1
	    .globl __bank_3_2

	    .globl __stub_0_1
	    .globl __stub_0_2
	    .globl __stub_0_3
	    .globl __stub_1_2
	    .globl __stub_1_3
	    .globl __stub_2_1
	    .globl __stub_2_3
	    .globl __stub_3_1
	    .globl __stub_3_2


            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (not unmapped when we flip to ROM)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

; FIXME: figure out how to reboot into CP/M
_trap_monitor:
_trap_reboot:
	    di
	    halt
platform_interrupt_all:
	    ret

;
;	FIXME: this probably needs to be a new "commondata" area
;
_kernel_flag:
	    .db 1

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ret

init_hardware:
            ; set system RAM size
	    ; Not strictly correct FIXME: set right when ROM the image
            ld hl, #64
            ld (_ramsize), hl
            ld hl, #32			; 32K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel
            ld hl, #0
            push hl
	    push af
            call _program_vectors
	    pop af
            pop hl

	    ; IRQ enables
	    ld a, #0xB		; OVF (timer), RXRDY (gapnio), 7508
	    out (0x04), a

	    push af
	    call _vtinit
	    pop af
            im 1 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM


_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off

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

	    ; our platform has a "true" common area, if it did not we would
	    ; need to copy the "common" code into the common area of the new
	    ; process.

	    ; falls through

	    ; Map functions are trivial for now
	    ; FIXME: will need to play with BANKR once we ROM the image
map_kernel: 
map_process:
map_process_always:
map_save:
map_restore:
	    ret	    

; outchar: Wait for UART TX idle, then print the char in A
outchar:
	    push af
outcharw:
	    in a, (0x15)
	    bit 0, a
	    jr z, outcharw
	    pop af
	    out (0x14), a	 
            ret

;
;	RAM disc
;
_hd_xferin:
;
;	Load the full 24 bit address to start
;
	xor a		;	 always aligned
	out (0x90), a
	ld hl, #_ramd_off + 1
	ld de, (_ramd_size)
hd_xiloop:
	ld a, (hl)
	out (0x91), a
	inc hl
	ld a, (hl)
	out (0x92), a
	ld bc, #0x93
;
;	With a 256 byte block it autoincrements
;
	ld hl, (_ramd_uaddr)
	inir
	dec d
	ret z
;
;	Move on a block
;
	ld (_ramd_uaddr), hl
	ld hl, #_ramd_off + 1
	inc (hl)
	jr nz, hd_xiloop
	inc hl
	inc (hl)
	dec hl
	jr hd_xiloop

_hd_xferout:
;
;	Load the full 24 bit address to start
;
	xor a		;	 always aligned
	out (0x90), a
	ld hl, #_ramd_off + 1
	ld de, (_ramd_size)
hd_xoloop:
	ld a, (hl)
	out (0x91), a
	inc hl
	ld a, (hl)
	out (0x92), a
	ld bc, #0x93
;
;	With a 256 byte block it autoincrements
;
	ld hl, (_ramd_uaddr)
	otir
	dec d
	ret z
;
;	Move on a block
;
	ld (_ramd_uaddr), hl
	ld hl, #_ramd_off + 1
	inc (hl)
	jr nz, hd_xoloop
	inc hl
	inc (hl)
	dec hl
	jr hd_xiloop


;
;	FIXME
current_map:
	.byte 0
switch_bank:
	ret
;
;
;	Banking helpers
;
;	FIXME: these are from zx128 - not yet converted to px4plus. We
; should also be able to dump all to/from bank2 versions
;
;
;	Logical		Physical
;	0		COMMON (0x4000)
;	1		0
;	2		1
;	3		7
;
;
__bank_0_1:
	xor a		   ; switch to physical bank 0 (logical 1)
bankina0:
	;
	;	Get the target address first, otherwise we will change
	;	bank and read it from the wrong spot!
	;
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	ld bc, (current_map)	; get current bank into B
	call switch_bank   ; Move to new bank
	; figure out which bank to map on the return path
	ld a, c
	or a
	jr z, __retmap1
	dec a
	jr z, __retmap2
	jr __retmap3

callhl:	jp (hl)
__bank_0_2:
	ld a, #1	   ; logical 2 -> physical 1
	jr bankina0
__bank_0_3:
	ld a, #7	   ; logical 3 -> physical 7
	jr bankina0

__bank_1_2:
	ld a, #1
bankina1:
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	call switch_bank   ; Move to new bank
__retmap1:
	ex de, hl
	call callhl	   ; call the function
	xor a		   ; return to bank 1 (physical 0)
	jp switch_bank

__bank_1_3:
	ld a, #7
	jr bankina1
__bank_2_1:
	xor a
bankina2:
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	call switch_bank   ; Move to new bank
__retmap2:
	ex de, hl
	call callhl	   ; call the function
	ld a, #1	   ; return to bank 2
	jp switch_bank
__bank_2_3:
	ld a, #7
	jr bankina2
__bank_3_1:
	xor a
bankina3:
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	call switch_bank   ; Move to new bank
__retmap3:
	ex de, hl
	call callhl	   ; call the function
	ld a, #7	   ; return to bank 0
	jp switch_bank

__bank_3_2:
	ld a, #1
	jr bankina3

;
;	Stubs need some stack munging and use DE
;

__stub_0_1:
	xor a
__stub_0_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discard
	ld bc, (current_map)
	call switch_bank
	ld a, c
	or a
	jr z, __stub_1_ret
	dec a
	jr z, __stub_2_ret
	jr __stub_3_ret
__stub_0_2:
	ld a, #1
	jr __stub_0_a
__stub_0_3:
	ld a, #7
	jr __stub_0_a

__stub_1_2:
	ld a, #1
__stub_1_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	call switch_bank
__stub_1_ret:
	ex de, hl
	call callhl
	xor a
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_1_3:
	ld a, #7
	jr __stub_1_a

__stub_2_1:
	xor a
__stub_2_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	call switch_bank
__stub_2_ret:
	ex de, hl	; DE is our target
	call callhl
	ld a,#1
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_2_3:
	ld a, #7
	jr __stub_2_a

__stub_3_1:
	xor a
__stub_3_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	call switch_bank
__stub_3_ret:
	ex de, hl
	call callhl
	ld a,#7
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_3_2:
	ld a, #1
	jr __stub_3_a
