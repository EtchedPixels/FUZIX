;
;    ZX Spectrum 128 hardware support
;

        .module zx128

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl platform_interrupt_all
	.globl interrupt_handler

        .globl map_kernel
        .globl map_process
        .globl map_process_always
        .globl map_save
        .globl map_restore

        .globl _kernel_flag

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

	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_0_3
	.globl __bank_1_2
	.globl __bank_1_3
	.globl __bank_2_1
	.globl __bank_2_3
	.globl __bank_3_1
	.globl __bank_3_2

        .include "kernel.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (below 0xC000)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	di
	halt

platform_interrupt_all:
        ret

_trap_reboot:
        rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

;
;	The memory banker will deal with the map setting
;
init_early:
;        ld bc, #0x7ffd
;        xor a
;        ld (current_map), a
;        out (c), a            ; set page 0 at 0xC000
        ret

	.area _VIDEO

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #(128 - 48)        ; 48K for kernel
        ld (_procmem), hl

        ; screen initialization
        ; clear
        ld hl, #0xC000
        ld de, #0xC001
        ld bc, #0x1800            ; There should be 0x17FF, but we are going
        xor a                     ; to copy additional byte to avoid need of
        ld (hl), a                ; DE and HL increment before attribute
        ldir                      ; initialization (2 bytes of RAM economy)

        ; set color attributes
        ld a, #7            ; black paper, white ink
        ld bc, #0x300 - #1
        ld (hl), a
        ldir

	; set up the interrupt vectors at 0xFFF4 in each bank we use for
	; kernel. This is a standard spectrum trick from the game world. The
	; interrupt vectors are in ROM and there is no mechanism to make
	; them call your own code. Instead we use IM2 and autovectors.
	; The spectrum bus value is not predictable so IM2 will jump through
	; a vector at I + (random 8bit value).
	; We point I at a chunk of empty ROM that holds 0xFF 0xFF .. for at
	; least 256 bytes. Our IRQ will jump to 0xFFFF which we set to be a
	; JR instruction. 0x0000 is a fixed ROM constant which forms a
	; backward jump to 0xFFF4, where we can finally grab the IRQ.
	;
	; We must keep the Spectrum 48K ROM image mapped at all times. The
	; 128K image hasn't got the 0xFF space we use!
	;
	call setallvectors
	ld a, #0x39
	ld i, a
;        im 2 ; set CPU interrupt mode
        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	pop bc
	pop de
	pop iy			;	task ptr
	push iy
	push de
	push bc
	ld a, P_TAB__P_PAGE_OFFSET+1(iy)	; high page of the pair
setvectors:
	call switch_bank
	ld a, #0x19
	ld (0xffff), a		;	JR (plus ROM at 0 gives JR $FFF4)
	ld a, #0xC3		;	JP
	ld (0xFFF4), a
	ld hl, #interrupt_handler
	ld (0xFFF5), a		;	to IRQ handler
	ret

setallvectors:
	call map_save
	xor a
	call setvectors
	ld a, #1
	call setvectors
	ld a, #7
	call setvectors
	jp map_restore

        ; bank switching procedure. On entrance:
        ;  A - bank number to set
	;
	; FIXME: we can probably stack BC now we are banking sanely
	;
switch_bank:
        di                  ; TODO: we need to call di() instead
        ld (current_map), a
        ld a, b
        ld (place_for_b), a
        ld a, c
        ld (place_for_c), a
        ld bc, #0x7ffd
        ld a, (current_map)
	or #0x18	   ; Spectrum 48K ROM, Screen in Bank 7
        out (c), a
        ld a, (place_for_b)
        ld b, a
        ld a, (place_for_c)
        ld c, a
        ld a, (place_for_a)
;FIXME      ei
        ret

map_kernel:
        ld (place_for_a), a
map_kernel_nosavea:          ; to avoid double reg A saving
        xor a
        jr switch_bank

map_process:
        ld (place_for_a), a
        ld a, h
        or l
        jr z, map_kernel_nosavea
        ld a, (hl)
        jr switch_bank

map_process_always:
        ld (place_for_a), a
        ld a, (U_DATA__U_PAGE)
        jr switch_bank

map_save:
        ld (place_for_a), a
        ld a, (current_map)
        ld (map_store), a
        ld a, (place_for_a)
        ret

map_restore:
        ld (place_for_a), a
        ld a, (map_store)
        jr switch_bank


; outchar: TODO: add something here (char in A). Current port #15 is emulator stub
outchar:
        out (#0x15), A
        ret
_kernel_flag:
        .db 1

	.area _COMMONDATA
current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 7ffd port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

place_for_a:                ; When change mapping we can not use stack since it is located at the end of banked area.
        .db 0               ; Here we store A when needed
place_for_b:                ; And BC - here
        .db 0
place_for_c:
        .db 0

	.area _COMMONMEM
;
;	Banking helpers
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
	call switch_bank   ; Move to new bank
	ex de, hl
	call callhl	   ; can't optimise - we need the stack depth right
	ret
callhl:	jp (hl)		   ; calls from bank 0 are different to the others
			   ; we started in common so we don't need to map
			   ; the old state back
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
	ex de, hl
	call callhl	   ; call the function
	ld a, #7	   ; return to bank 0
	jp switch_bank

__bank_3_2:
	ld a, #1
	jr bankina3
