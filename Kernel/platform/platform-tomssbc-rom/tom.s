
        .module tom

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
	.globl map_for_swap
	.globl map_bank_a
	.globl map_save_kmap
	.globl map_restore_kmap
	.globl map_buffers

	.globl current_map
	.globl switch_bank

        .globl _need_resched
	.globl _int_disabled

        ; exported debugging tools
        .globl _plt_monitor
	.globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

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

; Base address of SIO/2 chip 0x00

SIOA_D		.EQU	0x00
SIOB_D		.EQU	0x01
SIOA_C		.EQU	0x02
SIOB_C		.EQU	0x03


; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (above 0x4000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	di
	xor a
	out (0x3F),a
	out (0x3E),a
	out (0x38),a		; ROM in
	rst 0

plt_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0x4000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE1

;
;	The memory banker will deal with the map setting
;
init_early:
	ret

	.area _COMMONMEM

init_hardware:
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOA_C		; 10 bytes to SIOA_C
	otir

	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

        ; set system RAM size
        ld hl, #64		; 64K usable, other 64K is not wired
        ld (_ramsize), hl
        ld hl, #48		; 4 x 16K banked + 16K common + 32K
				; as we copy in/out as rather than exchange
        ld (_procmem), hl

	; Add the vectors to the low RAM
	call map_proc_always
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
	call map_kernel_restore

	im 1
	ret

RTS_LOW	.EQU	0xEA

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x19		; We want carrier events
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte RTS_LOW

;
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld b,#0x06
	ld c,l
	ld hl,#_sio_r
	otir
	ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}

map_bank_a:
        ; bank switching procedure. On entrance:
        ;  A - bank number to set
	push af
	ld a, (current_map)
	ld (ksave_map), a
	pop af
	; Then fall through to set the bank up

map_restore_kmap:
switch_bank:
	; Write the store first, that way any interrupt will restore
	; the new bank and our out will just be a no-op
        ld (current_map), a
	out (0x3F),a		; bank in bit 0
	rrca
	out (0x3E),a		; bank in bit 1
	rlca
	rlca			; ROM/RAM in high bit
        out (0x38), a
        ret

map_proc:
        ld a, h
        or l
        jr z, map_kernel_nosavea
	push af
        ld a, (hl)
	or #0x80		; ROM off
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
	or #0x80
	ld (current_map),a
	call switch_bank
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

; This will be matched by a map_kernel so save the current state so we put
; it back correctly
map_buffers:
	push af
	ld a,(current_map)
	ld (ksave_map),a
	pop af
	ret

map_restore:
	push af
        ld a, (map_store)
        call switch_bank
	pop af
	ret

map_save_kmap:
	ld a,(current_map)
	ret

; Map out ROM and ensure the matching map_kernel is ok
map_for_swap:
	push af
	ld a,(current_map)
	ld (ksave_map),a
	or #0x80
	call switch_bank
	pop af
	ret
;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
outchar:
	push af
	; wait for transmitter to be idle
ocloop_sio:
        xor a                   ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)		; read Line Status Register
	and #0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (SIOA_D),a
	ret

	.area _COMMONDATA
_tmpout:
	.db 1

current_map:                ; place to store current page number. Is needed
        .db 0x00            ; as we can't detect what page is mapped currently 
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
	ld a, #0x01	   ; logical 2 -> physical 1
	jr bankina0
__bank_0_3:
	ld a, #0x02	   ; logical 3 -> physical 2
	jr bankina0
__bank_0_4:
	ld a, #0x03	   ; logical 4 -> physical 3
	jr bankina0

__bank_1_2:
	ld a, #0x01
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
	xor a	   ; return to bank 1 (physical 0)
	jp switch_bank
__bank_1_3:
	ld a, #0x02
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
	ld a, #0x01	   ; return to bank 2
	jp switch_bank
__bank_2_3:
	ld a, #0x02
	jr bankina2
__bank_2_4:
	ld a, #0x03
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
	ld a, #0x02	   ; return to bank 3
	jp switch_bank
__bank_3_2:
	ld a, #0x01
	jr bankina3
__bank_3_4:
	ld a, #0x03
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
	ld a, #0x03	   ; return to bank 4
	jp switch_bank
__bank_4_2:
	ld a, #0x01
	jr bankina4
__bank_4_3:
	ld a, #0x02
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
	ld a, #0x01
	jr __stub_0_a
__stub_0_3:
	ld a, #0x02
	jr __stub_0_a
__stub_0_4:
	ld a, #0x03
	jr __stub_0_a

__stub_1_2:
	ld a, #0x01
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
	ld a, #0x02
	jr __stub_1_a
__stub_1_4:
	ld a, #0x03
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
	ld a,#0x01
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_2_3:
	ld a, #0x02
	jr __stub_2_a
__stub_2_4:
	ld a, #0x03
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
	ld a,#0x02
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_3_2:
	ld a, #0x01
	jr __stub_3_a
__stub_3_4:
	ld a, #0x03
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
	ld a, #0x01
	jr __stub_4_a
__stub_4_3:
	ld a, #0x02
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
;	We need these in _COMMONMEM
;
        .area _COMMONMEM

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

;
;	IDE helpers
;
	.globl _td_page
	.globl _td_raw

	.globl _devide_write_data
	.globl _devide_read_data

map:
	ld	bc,#IDE_REG_DATA
	ld	a,(_td_raw)
	or	a
	ret	z
	dec	a
	jp	z,map_proc_always
	ld	a,(_td_page)
	jp	map_for_swap

_devide_write_data:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	map
	otir
	otir
	jr	ide_out
_devide_read_data:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	map
	inir
	inir
ide_out:
	ld	a,(_td_raw)
	or	a
	ret	z
	jp	map_kernel
