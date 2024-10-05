;
;    ZX Spectrum 48K with DivIDE plus hardware support
;

        .module zx48

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler

        .globl map_kernel
        .globl map_proc_always
        .globl map_proc
        .globl map_kernel_di
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_proc_save
	.globl map_kernel_restore
	.globl map_buffers
	.globl map_bank_a
	.globl map_save_kmap
	.globl map_restore_kmap
	.globl current_map
	.globl switch_bank

        .globl _need_resched
	.globl _int_disabled

	.globl _machine_type
	.globl mapport
	.globl mapmod
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
        .include "../../cpu-z80/kernel-z80.def"

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
	di
	im 1
	; This sequence triggers the DivIDE Plus reset mechanism
	ld a,(_machine_type)
	or a
	jr nz, reboot_zxcf
	xor a
	ld i,a
	ld a,#0xC0
	out (0xE3),a
	halt
reboot_zxcf:
	di
	halt

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1
_machine_type:
	.db 1
mapmod:
	.dw 0x00E0		; for DivIDE Plus
mapport:
	.dw 0xFF17		; for DivIDE plus

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE1

;
;	The memory banker will deal with the map setting
;
init_early:
	ld (_machine_type),a
	or a
        ret z
	; Switch to mappings for ZXCF
	ld bc,#0x10BF
	ld (mapport),bc
	ld bc,#0x0240		; or with 0x40 move on by 2
	ld (mapmod),bc
	ret

	.area _COMMONMEM

init_hardware:
        ; set system RAM size
        ld hl, #560
	ld a,(_machine_type)
	or a
	jr nz, inith2
	ld hl,#528		  ; we lose bits to ResiDOS
inith2:
        ld (_ramsize), hl
	ld de, #112       ; 4 x 16K banked + 16K go to the OS/screen
			  ; and we lose 32K for speed so we can ldir
			  ; not to do tricky exchanges
	or a
	sbc hl,de
        ld (_procmem), hl

        ; screen initialization
	push af
	call _vtinit
	pop af

	; Vectors

	ld a,#4
vecloop:
	dec a
	push af
	call map_bank_a
	call vectors
	call map_kernel_restore
	pop af
	jr nz, vecloop
        ret

;	What's our vector, Victor ?

vectors:
	ld a,#0xC3
	ld (0),a
	ld hl,#null_handler
	ld (1),hl
	ld (0x30),a
	ld hl,#unix_syscall_entry
	ld (0x31),hl
	ld (0x38),a
	ld hl,#interrupt_handler
	ld (0x39),hl
	ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}

map_buffers:
	ld a,#2
map_restore_kmap:
map_bank_a:
        ; bank switching procedure. On entrance:
        ;  A - bank number to set
	push af
	ld a, (current_map)
	ld (ksave_map), a
	call switch_bank
	pop af
	ret

map_save_kmap:
	ld a,(current_map)
	ret

switch_bank:
	push bc
	; Write the store first, that way any interrupt will restore
	; the new bank and our out will just be a no-op
        ld (current_map), a
	ld bc,(mapmod)		; 0x60 for DivIDE plus 0x40 for ZXCF+
	add b			; have to avoid bank 0 on ZXCF+ (residos)
	or c
	ld bc,(mapport)		; 0x17 for DivIDE plus 0x4278 for ZXCF+
        out (c), a
	pop bc
        ret

map_proc:
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
map_proc_save:
map_proc_always:
map_proc_always_di:
	push af
	ld a, (current_map)
	ld (ksave_map), a
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
;	callers who do map_proc_always/map_kernel pairs. Probably we
;	should have some global change to map_proc_save/map_kernel_restore
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

	.area _COMMONDATA
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

	.area _COMMONMEM
;
;	Banking helpers
;
;	Logical		Physical
;	0		COMMON (0x4000)
;	1		0
;	2		1
;	3		2
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
	dec a
	jr z, __retmap3
	jr __retmap4

callhl:	jp (hl)
__bank_0_2:
	ld a, #1	   ; logical 2 -> physical 1
	jr bankina0
__bank_0_3:
	ld a, #2	   ; logical 3 -> physical 2
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
	ld a, #2
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
	ld a, #2
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
	ld a, #2	   ; return to bank 3
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
	ld a, #2
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
	or a
	jr z, __stub_1_ret
	dec a
	jr z, __stub_2_ret
	jr __stub_3_ret
__stub_0_2:
	ld a, #1
	jr __stub_0_a
__stub_0_3:
	ld a, #2
	jr __stub_0_a
__stub_0_4:
	ld a, #3
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
	push de		; FIXME don't we need to use BC and can't we get
	ret		; rid of all non 0_x stubs ?
__stub_1_3:
	ld a, #2
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
	ld a, #2
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
	ld a,#2
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
	ld a, #2
	jr __stub_4_a

;
;	We know that we only ever copy between the user space and 4000-7FFF
;	or the buffers (0000-3FFF in our bank). So if we put our copiers in
;	the right bank and not common space we can just ldir stuff about
;	which is far simpler
;
        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_proc_always
	.globl  map_kernel
;
;	We need these in _CODE3 so they are in the right bank to copy disk
;	cache buffers
;
        .area _CODE3

;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
uputget:
        ; load DE with the byte count
        ld c, 10(ix) ; byte count
        ld b, 11(ix)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, 6(ix) ; src address
        ld h, 7(ix)
        ; load DE with destination address (in userspace)
        ld e, 8(ix)
        ld d, 9(ix)
	ret	; 	Z is still false

;
; We should use add hl,sp logic here as I think it's faster
;
__uputc:
	pop iy	;	bank
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop iy
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	call map_proc_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
	call map_proc_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
uput_out:
	call map_kernel
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
	jr uput_out

;
__uzero:
	pop iy
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl	
	push de
	push iy
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_always
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out
