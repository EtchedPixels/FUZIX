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
	.globl map_process_save
	.globl map_kernel_restore
	.globl current_map

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
        im 2 ; set CPU interrupt mode
        ret

	.area _VIDEO

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #(128 - 64)        ; 64K for kernel/screen/etc
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
	call map_save
	call switch_bank
	ld a, #0x18
	ld (0xffff), a		;	JR (plus ROM at 0 gives JR $FFF4)
	ld a, #0xC3		;	JP
	ld (0xFFF4), a
	ld hl, #interrupt_handler
	ld (0xFFF5), hl		;	to IRQ handler
	call map_restore
	ret

setallvectors:
	xor a
	call setvectors
	ld a, #1
	call setvectors
	ld a, #7
	jp setvectors

        ; bank switching procedure. On entrance:
        ;  A - bank number to set

switch_bank:
	; Write the store first, that way any interrupt will restore
	; the new bank and our out will just be a no-op
        ld (current_map), a
	push bc
        ld bc, #0x7ffd
	or #0x18	   ; Spectrum 48K ROM, Screen in Bank 7
        out (c), a
	pop bc
        ret


;
;	These are incomplete - or at least someone somewhere has to do the
;	exchange hackery as we've only got a 16K window so can't flip 0x8000
;	directly.
;
map_process:
        ld a, h
        or l
        jr z, map_kernel_nosavea
	push af
        ld a, (hl)
	call switch_bank
	pop af
	ret

map_process_save:
	push af
	ld a, (current_map)
	ld (ksave_map), a
	pop af

map_process_always:
	push af
	ld a, (current_map)
	ld (ksave_map), a
        ld a, (U_DATA__U_PAGE)
	call switch_bank
	pop af
	ret

;
;	This may look odd. However the kernel is banked so any
;	invocation of kernel code in fact runs common code and the
;	common code will bank in the right kernel bits for us when it calls
;	out of common into banked code. We do a restore to handle all the
;	callers who do map_process_always/map_kernel pairs. Probably we
;	should have some global change to map_process_save/map_kernel_restore
;
map_kernel:
map_kernel_nosavea:          ; to avoid double reg A saving
map_kernel_restore:
	push af
	ld a, (ksave_map)
	call switch_bank
	pop af
	ret

map_save:
	push af
        ld a, (current_map)
        ld (map_store), a
	pop af
        ret

map_restore:
	push af
        ld a, (map_store)
        call switch_bank
	pop af
	ret


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

ksave_map:
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

;
;	Stubs from common are the easy case and use HL
;
__stub_0_1:
	xor a
	call switch_bank
	jp (hl)

__stub_0_2:
	ld a,#1
	call switch_bank
	jp (hl)

__stub_0_3:
	ld a,#7
	call switch_bank
	jp (hl)

;
;	Other stubs need some stack munging and use DE 
;
__stub_1_2:
	ld a, #1
__stub_1_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	call switch_bank
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
