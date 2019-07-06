;
;    ZX Uno hardware support
;

        .module zxuno

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl platform_interrupt_all
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
	.globl map_save_kmap	
	.globl map_restore_kmap
	.globl map_process_a
	.globl map_video_save
	.globl current_map
	.globl switch_bank

        .globl _need_resched
	.globl _int_disabled
	.globl _portff
	.globl ksave_map

        ; exported debugging tools
        .globl _platform_monitor
	.globl _platform_reboot
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
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (below 0x4000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_platform_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _platform_monitor

_platform_reboot:
	di
	im 1
	ld bc, #0x7ffd
	xor a		; 128K ROM, initial banks, low screen
	out (c), a
        rst 0		; Into the ROM

platform_interrupt_all:
        ret

	.area _COMMONDATA

_int_disabled:
	.db 1


; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 0x4000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE1

;
;	The memory banker will deal with the map setting
;
init_early:
        ret

	.area _COMMONMEM

init_hardware:
        ; set system RAM size
	; sort of anyway - we need to revisit this with the real values
	; once we have the rest of the map worked out
        ld hl, #256
        ld (_ramsize), hl
        ld hl, #(192)        ; 64K for kernel/screen/etc
        ld (_procmem), hl

        ; screen initialization
	ld a,(_portff)
	out (0xff),a

	push af
	call _vtinit
	pop af

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_vectors:
	ret

switch_bank:
	; Write the store first, that way any interrupt will restore
	; the new bank and our out will just be a no-op
        ld (current_map), a
	push bc
        ld bc, #0x7ffd
	or #BANK_BITS	   ; Spectrum 48K ROM, Screen in Bank 7
        out (c), a
	pop bc
        ret

;
;	On the ZX Uno we have the dock and ext banks we can use for
;	user space work
;
;	Our maps are
;
;	1	2,5,4 base
;	2	EXROM
;	3	DOCK
;
;	Always set an even top 16K page because of the overlap rules
;	(TODO: check the uno suffers from them)
;
map_process:
        ld a, h
        or l
        jr z, map_kernel_nosavea
	push af
        ld a, #4		; Top bank should be 4 for user space
	call switch_bank
	ld a,(hl)
do_map_process_a:
	ld (user_map),a
	dec a
	jr z, map_process_base	; Map 1 = main memory
	dec a
	jr z, map_exrom		; Map 2 = exrom
	;
	;	We are mapping in the dock
	;
	ld a,(_portff)		; Switch external banks to the dock
	and #0x7f		; As we already set our high bank to 4
	ld (_portff),a		; this will work as expected
	out (0xff),a
	ld a,#0xF8		; Map the dock into the top 40K
	out (0xF4),a
	pop af
	ret
map_exrom:
	ld a,(_portff)		; Switch external banks to the exrom
	or #0x80		; which in this case is actually RAM
	ld (_portff),a
	out (0xff),a
	ld a,#0xF8		; Top 40K again for now
	out (0xF4),a
	pop af
	ret
map_process_base:		; We want main RAM, we don't care about
	xor a			; the current dock/exrom bit
	out (0xf4),a		; just set all RAM to internal
	pop af
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
	; Now do the user mapping
map_process_a:
	push af
        ld a, #4		; Top bank should be 4 for user space
	call switch_bank
	pop af
	push af
	jr do_map_process_a
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
	ld a, #4
	call switch_bank
        ld a, (_udata + U_DATA__U_PAGE)
	jr do_map_process_a

;
;	Save and switch to kernel
;
map_save_kernel:
	push af
	ld a, (user_map)
	ld (user_store),a
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
	xor a
	ld (user_map),a
	out (0xF4),a
	ld a, (ksave_map)
	call switch_bank
	pop af
	ret

;
;	Temporarily switch to video memory, this lives in bank 7 so in the
;	the same space as the kernel banks. We are in common, the video code
;	is in common and the fonts too (but need moving to bank 7)
;
map_video_save:
	push af
	ld a,(current_map)
	ld (ksave_map),a
	ld a,#7
	call switch_bank
	pop af
	ret

map_restore:
	push af
	ld a, (user_store)
	ld (user_map),a
	or a
	jp nz, do_map_process_a
krestore:
	xor a
	out (0xF4),a
	ld (user_map),a
	ld a, (map_store)
	call switch_bank
	pop af
	ret

map_save_kmap:
	ld a, (current_map)
	ret

map_restore_kmap:
	ld (ksave_map),a
	jp map_kernel_di

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
user_map:
	.db 0
map_store:
        .db 0
user_store:
	.db 0

ksave_map:
        .db 0

_portff:
	.db 0x3E	    ; High resolution white on black

	.area _COMMONMEM
;
;	Banking helpers
;
;	Logical		Physical
;	0		COMMON (0x4000)
;	1		0
;	2		1
;	3		6
;
;	(7 is the screens and pretty full)
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
	ld a, #6	   ; logical 3 -> physical 6
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
	ld a, #6
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
	ld a, #6
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
	ld a, #6	   ; return to bank 0
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
	ld a, #6
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
	ld a, #6
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
	ld a, #6
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
	ld a,#6
	call switch_bank
	pop de
	push de		; dummy the caller will discard
	push de
	ret
__stub_3_2:
	ld a, #1
	jr __stub_3_a
