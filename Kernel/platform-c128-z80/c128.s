;
;	Minimal C128 support
;
        .module c128

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_process_di
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl map_buffers
	.globl plt_interrupt_all
	.globl _plt_reboot
	.globl _plt_monitor
	.globl _bufpool
	.globl _int_disabled

        ; imported symbols
	.globl init
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl nmi_handler
	.globl null_handler

	; exported debugging tools
	.globl outchar

        .include "kernel.def"
        .include "../kernel-z80.def"

;=========================================================================
; Buffers
;=========================================================================
        .area _BUFFERS
	.globl kernel_endmark

_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this
;
;	So we can check for overflow
;
kernel_endmark:

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
	ld	hl,#128
	ld	(_ramsize), hl
	ld	hl,#60
	ld	(_procmem), hl
	ld	hl,#null_handler
	ld	(1),hl
	ld	hl,#interrupt_handler
	ld	(0x39),hl
	ld	hl,#nmi_handler
	ld	(0x67),hl
	ld	a,#0xC3
	ld	(0),a
	ld	(0x38),a
	ld	(0x66),a
	ret

;=========================================================================
; Common Memory (mapped low below ROM)
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
	; TODO we have a real monitor!
_plt_reboot:
	di
	halt

;=========================================================================

_int_disabled:
	.db 1

; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
	ret

;=========================================================================
; Memory management
;=========================================================================

; TODO - use preloads for these and save the stacking

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
map_process_di:
	ld a,h
	or l				; HL == 0?
	jr z,map_kernel			; HL == 0 - map the kernel

	; fall through

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
;=========================================================================
map_for_swap:

	; fall through

;=========================================================================
; map_process_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_process_always:
map_process_always_di:
	push	af
	ld	a,#0x3F		; Process in bank 0
	ld	(map),a
	ld	(0xFF00),a
	pop	af
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
map_kernel:
map_kernel_restore:
map_kernel_di:
	push	af
	ld	a,#0x7F		; Kernel in bank 1
	ld	(map),a
	ld	(0xFF00),a
	pop	af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push af
	ld a,(saved_map)
	ld (map),a
	ld (0xFF00),a
	pop af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push af
	ld a,(map)
	ld (saved_map),a
	ld a,#0x7F
	ld (0xFF00),a
	pop af
	ret

map:	.byte	0x7F
saved_map:
	.byte	0x7F

;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	; TODO - use video helper ?
	ret

	.globl _rd_dptr
	.globl _rd_page
	.globl _rd_block

_rd_dptr:
	.word	0
_rd_page:
	.byte	0
_rd_block:
	.word	0

geo_setup:
	ld	a,(_rd_page)
	or	a
	call	nz, map_process_always
	ld	hl,(_rd_block)
	; As an address this looks like
	;	0BBBBBBB BPPPPPPX [XXXXXXXX]
	ld	a,l
	add	a
	sla	h	;	H is now the 16K block number
	rrca
	rrca
	and	#0x3f	;	A is the page
	ld	bc,#0xdfff
	out	(c),h	;	16K bank
	dec	c
	out	(c),a	;	A is the page
	ld	hl, (_rd_block)
	ld	de , #0xDE00
	ld	bc, #256
	inc	a	;	next page - used during copy
	;	BC DE HL set up for copy, A is the next page
	;	We never cross banks in one I/O
	ret

	.globl	_geo_read
_geo_read:
	call	geo_setup
	ldir
	ld	bc,#0xDFFE
	out	(c),a
	;	now inc the page
	ld	bc,#256
	;	back a page
	dec	d
	ldir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.globl	_geo_write
_geo_write:
	call	geo_setup
	ex	de,hl
	ldir
	ld	bc,#0xDFFE
	out	(c),a
	;	now inc the page
	ld	bc,#256
	;	back a page
	dec	d
	ldir
	ld	(_rd_dptr),hl
	jp	map_kernel

	.globl _reu_read

_reu_read:
	ld	a,#0x91
	.byte	0x01		; ld bc.. skip the write load
	.globl _reu_write
_reu_write:
	ld	a,#0x90
reu_execute:
	push	af
	ld	a,(_rd_page)
	or	a
	call	nz, map_process_always
	ld	hl,(_rd_dptr)
	ld	bc,#0xDF02
	out	(c),l
	inc	c
	out	(c),h		; Set C128 ptr
	;	Now work out the block address
	xor	a
	out	(c),a		; Block address low is always 0
	ld	hl,(_rd_block)
	add	hl,hl		; turn 512 byte sectors into address in 256's
	inc	c
	out	(c),l
	inc	c
	out	(c),h
	ld	de,#512		; Set length
	inc 	c
	out	(c),e
	inc	c
	out	(c),d
	ld	hl,(_rd_dptr)
	add	hl,de		; Get final dptr for return
	ld	(_rd_dptr),hl
	xor	a
	out	(c),a		; Not sure we need this
	ld	c,#1		; command
	pop	af
	out	(c),a
	jp	map_kernel

	.area _DISCARD

	; Detect and size the RAM disc options
	.globl	_reu_probe
_reu_probe:
	xor	a		; Do a compare at kernel AA55
	ld	(_rd_page),a	; if it is an REU then we'll end up AC55
	ld	hl,#0xAA55
	ld	(_rd_dptr),hl
	ld	a,#0x93
	call	reu_execute
	ld	c,#2
	in	a,(c)
	cp	#0x55
	jr	nz, not_reu
	inc	c
	in	a,(c)
	cp	#0xAC
	jr	nz, not_reu
	ld	l,#1
	;	TODO size the REU by writing a number to each boundary
	;	and seeing when it breaks or repeats
	ret
not_reu:
not_geo:
	ld	l,#0
	ret

	.globl	_geo_probe

geopat:
	ld	bc,#0xDFFF
	out	(c),a
	dec	c
	xor	a
	out	(c),a
	; Selected block 0
	ld	bc,#0xDE00
	out	(c),l
	inc	c
	out	(c),h
	ret

; Read first byte of the first bank
georead:
	ld	bc,#0xDFFF
	xor	a
	out	(c),a		; Bank L,0
	dec	c
	out	(c),a
	ld	bc,#0xDE00
	in	a,(c)
	ret

; Label a bank with its own number
geowrite:
	ld	bc,#0xDFFF
	out	(c),l
	xor	a
	dec	c
	out	(c),a
	ld	bc,#0xDE00
	out	(c),l
	ret

_geo_probe:
	; Georam is a simple banked paged RAM at $DExx controlled by $DFFx
	xor	a
	ld	hl,#0xAA55		; Write AA55
	call	geopat
	ld	a,#1
	ld	hl,#0x55AA		; Write 55AA in a different page
	call	geopat
	xor	a			; read the initial word back
	ld	bc,#0xDFFF
	out	(c),a
	inc	c
	out	(c),a
	ld	bc,#0xDE00
	in	a,(c)
	cp	#0xAA			; shoudl be AA 55 if not something else
	jr	nz, not_geo
	inc	c
	in	a,(c)
	cp	#0x55
	jr	nz, not_geo
	ld	l,#0
geo_next:
	call	geowrite
	call	georead
	or	a
	jr	nz, geo_wrap
	inc	l
	jr	nz, geo_next
	; We found the full 4MB */
	ld	hl,#8192		; 8192 blocks
	ret
geo_wrap:
	; We found l 16K banks
	ld	h,#0
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ret
