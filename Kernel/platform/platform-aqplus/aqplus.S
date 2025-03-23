        ; exported symbols
        .export init_hardware
	.export _program_vectors
	.export map_kernel
	.export map_kernel_restore
	.export map_proc
	.export map_buffers
	.export map_kernel_di
	.export map_proc_di
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap
	.export plt_interrupt_all
	.export _copy_common
	.export mpgsel_cache
	.export top_bank
	.export _kernel_pages
	.export _plt_reboot
	.export _plt_monitor
	.export _int_disabled
	.export _plt_idle

	.export _iop_data_in
	.export _iop_data_out

	; exported debugging tools
	.export outchar

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;=========================================================================
; Initialization code
;=========================================================================
        .discard

init_hardware:
        ld hl, 0
        push hl
        call _program_vectors
        pop hl

	; Compiler helper vectors - in kernel bank only

	ld	hl,rstblock
	ld	de,8
	ld	bc,32
	ldir

	; Get the internal DI state right
	call ___hard_di

	ret


;=========================================================================
; Kernel code
;=========================================================================
	.code

_plt_monitor:
_plt_reboot:
        di
_plt_idle:
	halt
	ret

;=========================================================================
; Common Memory (0xF000 upwards)
;=========================================================================
	.common

;=========================================================================

_int_disabled:
	.byte 1

plt_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_proc

	ld a,0xC3			; JP instruction
	ld (0x0038),a
	ld hl,interrupt_handler
	ld (0x0039),hl

	ld (0x0000),a
	ld hl,null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; Memory management
; - kernel pages:     32 - 34
; - common page:      35 (also init top page)
; - user space pages: 36 - 63
;=========================================================================

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
map_proc_always_di:
	push hl
	ld hl,_udata + U_DATA__U_PAGE
        jr map_proc_2_pophl_ret

;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
	ld a,h
	or l				; HL == 0?
	jr nz,map_proc_2		; HL == 0 - map the kernel

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_kernel_di:
map_kernel_restore:
map_buffers:
	push hl
	ld hl,_kernel_pages
        jr map_proc_2_pophl_ret

;=========================================================================
; map_proc_2 - map process or kernel pages
; Inputs: page table address in HL
; Outputs: none, HL destroyed
;=========================================================================
map_proc_2:
	push de
	push af

	ld de,mpgsel_cache		; paging registers are write only
					; so cache their content in RAM
	ld a,(hl)			; memory page number for bank 0
	ld (de),a
	out (MPGSEL_0),a		; set bank 0
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank 1
	ld (de),a
	out (MPGSEL_1),a		; set bank 1
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank 2
	ld (de),a
	out (MPGSEL_2),a		; set bank 2

	pop af
	pop de
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push hl
	ld hl,map_savearea
map_proc_2_pophl_ret:
	call map_proc_2
	pop hl
	ret

;=========================================================================
; map_save_kernel - save the current page mapping to map_savearea and
; switch to kernel maps
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push hl
	ld hl,(mpgsel_cache)
	ld (map_savearea),hl
	ld hl,(mpgsel_cache+2)
	ld (map_savearea+2),hl
	ld hl,_kernel_pages
	jr map_proc_2_pophl_ret

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
; We use 0x4000-0x7FFF so that all the interrupt stuff is mapped.
;
;=========================================================================
map_for_swap:
	ld (mpgsel_cache + 1),a
	out (MPGSEL_1),a
	ret

_copy_common:
	pop hl
	pop de
	push de
	push hl
	ld a,e
	call map_for_swap
	ld hl,0xEA00
	ld de,0x2A00
	ld bc,0x1600
	ldir
	jr map_kernel


; MPGSEL registers are write only, so their content is cached here
mpgsel_cache:
	.byte	0x3C,0x3D,0x3E
top_bank:	; the shared tricks code needs this name for cache+3
	.byte	0x3F

; kernel page mapping
_kernel_pages:
	.byte	0x3C,0x3D,0x3E,0x3F

; memory page mapping save area for map_save/map_restore
map_savearea:
	.byte	0,0,0,0

;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	; TODO
	push af
	pop af
	ret

;	Stub helpers for code compactness.

	.discard

rstblock:
; TODO


;
;	Disk I/O helpers
;

mapio:
	ld bc,0xF5		; set up for xfer
	ld a,(_td_raw)
	or a
	ret z			; Kernel map
	dec a
	jp z, map_proc_always	; User map
	ld a,(_td_page)
	jp map_for_swap		; Swap map

_iop_data_in:
	call mapio
	push bc
	call iorx
	call iorx
	pop bc
	jp map_kernel

iorx:
	in a,(0xF4);
	rra
	jr nc, iorx
	ini
	jr nz,iorx
	ret

_iop_data_out:
	call mapio
	call iotx
	call iotx
	jp map_kernel

iotx:
	in a,(0xF4)
	and 2
	jr nz, iotx
	outi
	jr nz, iotx
	ret
