;
;	128K ZX Spectrum with a Spectranet providing a bit under 12K of
;	memory we can use at 0x1000 to 0x39FF.
;
;	Kernel is banked into
;	0		CODE 1
;	1		CODE 2 + BUFFERS
;	3		CODE 4
;	7		CODE 3 + VIDEO + FONTS
;	2/5		4000-BFFF (common and user low buffer)
;
;	4		Free
;	8+		Free
;

        .module zx128

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler

        .globl map_kernel
        .globl map_process_always
        .globl map_process
        .globl map_kernel_di
        .globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_process_save
	.globl map_kernel_restore
	.globl map_for_swap
	.globl current_map
	.globl switch_bank

        .globl _need_resched
	.globl _int_disabled
	.globl _udata

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

	.globl null_handler
	.globl nmi_handler
	.globl unix_syscall_entry
	.globl interrupt_handler

	; banking support
	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_0_3
	.globl __bank_0_4
	.globl __bank_1_2
	.globl __bank_1_3
	.globl __bank_1_4
	.globl __bank_2_1
	.globl __bank_2_3
	.globl __bank_2_4
	.globl __bank_3_1
	.globl __bank_3_2
	.globl __bank_3_4
	.globl __bank_4_1
	.globl __bank_4_2
	.globl __bank_4_3

	.globl __stub_0_1
	.globl __stub_0_2
	.globl __stub_0_3
	.globl __stub_0_4
	.globl __stub_1_2
	.globl __stub_1_3
	.globl __stub_1_4
	.globl __stub_2_1
	.globl __stub_2_3
	.globl __stub_2_4
	.globl __stub_3_1
	.globl __stub_3_2
	.globl __stub_3_4
	.globl __stub_4_1
	.globl __stub_4_2
	.globl __stub_4_3

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (below 0xC000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _plt_monitor

_plt_reboot:
	; FIXME: need to turn the Spectranet off
	di
	im	1
	ld	bc, #0x7ffd
	xor	a		; 128K ROM, initial banks, low screen
	out	(c), a
        rst	0		; Into the ROM (SpectraNet will do the rest)

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1


; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE1

;
;	The memory banker will deal with the map setting
;
init_early:
	xor a
	out (0xFE),a
        ret

	.area _COMMONMEM

init_hardware:
        ; set system RAM size
	; Complicated question - what exactly does this mean on a spectranet
        ; 8)
        ld	hl, #256
        ld	(_ramsize), hl
	; We lose the following to the system
	; 0: first kernel bank at C000
	; 1: second kernel bank at C000
	; 2: 8000-BFFF (screen and buffers)
	; 3: fourth kernel bank at C000
	; 5: 4000-7FFF (working 16K copy)
	; 7: third kernel bank at C000
	; TODO: figure out the right value!
        ld	hl, #(256 - 96)
        ld	(_procmem), hl

	;
	;	No low RAM so need IM2
	;
	ld	b,#8
set_vectors:
	push	bc
	ld	a,b
	dec	a
	call	switch_bank
	ld	a,#0x18		; JR
	ld	(0xFFFF),a
	ld	a,#0xC3
	ld	(0xFFF4),a
	ld	hl,#interrupt_handler
	ld	(0xFFF5),hl
	pop	bc
	djnz	set_vectors

	ld	a,(current_map)
	call	switch_bank

        ; screen initialization
	push	af
	call	_vtinit
	pop	af

	; interrupt vectors to FFFF
	ld	hl,#0x1000
	ld	bc,#0xFF
intvector:
	ld	(hl),c
	inc	hl
	djnz	intvector
	ld	(hl),c

	ld	a,#0x10
	ld	i,a
	im	2		; Everything ends up at FFF4

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

	; Done at boot up
_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}
map_for_swap:
        ; bank switching procedure. On entrance:
        ;  A - bank number to set
	push af
	ld a, (current_map)
	ld (ksave_map), a
	pop af
	; Then fall through to set the bank up

switch_bank:
	; Write the store first, that way any interrupt will restore
	; the new bank and our out will just be a no-op
        ld (current_map), a
	push bc
        ld bc, #0x7ffd
	or #BANK_BITS	   ; Spectrum 48K ROM, high video
        out (c), a
	pop bc
        ret

;
;	TODO: if we go beyond the upper 32K we need to set the spectranet
;	map to (hl + 2)
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

;
;	We always save here so that existing code works until we have a
;	clear usage of save/restore forms across the kernel
;
map_process_save:
map_process_always:
map_process_always_di:
	push af
	ld a, (current_map)
	ld (ksave_map), a
        ld a, (_udata + U_DATA__U_PAGE)
	call switch_bank
	; Will need to also set spectranet banking to U_PAGE2 in future
	pop af
	ret

;
;	Save and switch to kernel
;
map_save_kernel:
	push af
        ld a, (current_map)
        ld (map_store), a
	pop af
;
;	This may look odd. However the kernel is banked so any
;	invocation of kernel code in fact runs common code and the
;	common code will bank in the right kernel bits for us when it calls
;	out of common into banked code. We do a restore to handle all the
;	callers who do map_process_always/map_kernel pairs. Probably we
;	should have some global change to map_process_save/map_kernel_restore
;
map_kernel_di:
map_kernel:
map_kernel_nosavea:          ; to avoid double reg A saving
map_kernel_restore:
	push af
	ld a, (ksave_map)
	call switch_bank
	pop af
	ret


map_restore:
	push af
        ld a, (map_store)
        call switch_bank
	pop af
	ret

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
	push af
	call _vtoutput
	pop af
	pop af
	pop af
	pop ix
	pop hl
	pop de
	pop bc
        ret

_tmpout:
	.db 1

current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 7ffd port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

ksave_map:
        .db 0

_need_resched:
        .db 0

;
;	Banking helpers
;
;	Logical		Physical
;	0		COMMON (0x4000)
;	1		0
;	2		1
;	3		7
;	4		3
;
;
__bank_0_1:
	xor a		   ; switch to physical bank 1 (logical 1)
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
	ld bc, (current_map)	; get current bank into C
	call switch_bank   ; Move to new bank
	; figure out which bank to map on the return path
	ld a, c
	or a			; Physical bank was 0 so CODE1
	jr z, __retmap1
	dec a			; Physucal bank was 1 so CODE2
	jr z, __retmap2
	cp #2			; Physical bank was 3 so CODE4
	jr z, __retmap4
	jr __retmap3		; Must be CODE3 (7)

callhl:	jp (hl)

__bank_0_2:
	ld a, #1	   ; logical 2 -> physical 1
	jr bankina0
__bank_0_3:
	ld a, #7	   ; logical 3 -> physical 7
	jr bankina0
__bank_0_4:
	ld a, #3	   ; logical 4 -> physical 3
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
__bank_1_4:
	ld a, #3
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
__bank_2_4:
	ld a, #3
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
	ld a, #7	   ; return to bank 3
	jp switch_bank
__bank_3_2:
	ld a, #1
	jr bankina3
__bank_3_4:
	ld a, #3
	jr bankina3

__bank_4_1:
	xor a
bankina4:
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	call switch_bank   ; Move to new bank
__retmap4:
	ex de, hl
	call callhl	   ; call the function
	ld a, #3	   ; return to bank 4
	jp switch_bank
__bank_4_2:
	ld a, #1
	jr bankina4
__bank_4_3:
	ld a, #7
	jr bankina4


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
	or a		; bank 0 (logical 1)
	jr z, __stub_1_ret
	dec a		; bank 1 (logical 2)
	jr z, __stub_2_ret
	cp #2		; bank 3 (logical 4)
	jr z, __stub_4_ret
	jr __stub_3_ret	; bank 7 (logical 3)
__stub_0_2:
	ld a, #1
	jr __stub_0_a
__stub_0_3:
	ld a, #7
	jr __stub_0_a
__stub_0_4:
	ld a, #3
	jr __stub_0_a

__stub_1_2:
	ld a, #1
__stub_1_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discard
	call switch_bank
__stub_1_ret:
	ex de, hl
	call callhl
	xor a
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de		; FIXME don't we need to use BC and can't we get
	ret		; rid of all non 0_x stubs ?
__stub_1_3:
	ld a, #7
	jr __stub_1_a
__stub_1_4:
	ld a, #3
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
__stub_2_4:
	ld a, #3
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
__stub_3_4:
	ld a, #3
	jr __stub_3_a

__stub_4_1:
	xor a
__stub_4_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	call switch_bank
__stub_4_ret:
	ex de, hl
	call callhl
	ld a,#3
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_4_2:
	ld a, #1
	jr __stub_4_a
__stub_4_3:
	ld a, #7
	jr __stub_4_a
