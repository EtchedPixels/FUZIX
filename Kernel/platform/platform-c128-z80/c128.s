
;
;	Minimal C128 support
;
        .module c128

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_proc_di
	.globl map_proc_always_di
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
        .include "../../cpu-z80/kernel-z80.def"

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
	ld	(0xFDFE),hl
	ld	hl,#nmi_handler
	ld	(0x67),hl
	ld	a,#0xC3
	ld	(0),a
	ld	(0x38),a
	ld	(0xFDFD),a
	ld	(0x66),a
	;	For now shut up the CIA
	ld	bc,#0xDC0D
	ld	a,#0x7F
	out	(c),a
	in	a,(c)
	inc	b
	;	And CIA2
	ld	a,#0x7F
	out	(c),a
	in	a,(c)
	ret

;=========================================================================
; Common Memory (mapped at F000 upwards)
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

;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
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
; map_proc_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
map_proc_always_di:
	push	af
	ld	a,#0x7F		; Process in bank 1
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
	ld	a,#0x3F		; Kernel in bank 0
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
	push	af
	ld	a,(saved_map)
	ld	(map),a
	ld	(0xFF00),a
	pop	af
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push	af
	ld	a,(map)
	ld	(saved_map),a
	ld	a,#0x3F
	ld	(map),a
	ld	(0xFF00),a
	pop	af
	ret

;
; C128 specific. Map the I/O space from kernel
;
map_io:
	push	af
	ld	a,#0x3E
	ld	(map),a
	ld	(0xFF00),a
	pop	af
	ret

map:	.byte	0x3F
saved_map:
	.byte	0x3F

;
;	I/O helpers. This is ugly. The C128 has memory mapped I/O at Dxxx
;	The Z80 can access this via in and out but accesses to non existent
;	ranges go to memory so it's useless for probing and some other
;	activities.
;
ioread8:
	call	map_io
	ld	a,(bc)
	jp	map_kernel
iowrite8:
	call	map_io
	ld	(bc),a
	jp	map_kernel
ioread16:
	call	map_io
	ld	a,(bc)
	ld	l,a
	inc	bc
	ld	a,(bc)
	ld	h,a
	jp	map_kernel
iowrite16:
	call	map_io
	ld	a,l
	ld	(bc),a
	inc	bc
	ld	a,h
	ld	(bc),a
	jp	map_kernel
;
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
	call	nz, map_proc_always
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
	ld	de, #0xDE00
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
	call	nz, map_proc_always
	ld	hl,(_rd_dptr)
	ld	bc,#0xDF02
	out	(c),l
	inc	c
	out	(c),h		; Set C128 ptr
	inc	c
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
	inc	c
	ld	hl,(_rd_dptr)
	add	hl,de		; Get final dptr for return
	ld	(_rd_dptr),hl
	xor	a
	out	(c),a		; Not sure we need this
	ld	c,#1		; command
	pop	af
	out	(c),a
	jp	map_kernel

	;	.area _DISCARD
	.area _CODE

	; Detect and size the RAM disc options
	.globl	_reu_probe

;
;	Read or write byte DE0.24 REU to/from HL
;
_reu_getb:
	ld	a,#0x91
	jr	_reu_getput
_reu_putb:
	ld	a,#0x90
_reu_getput:
	ld	(0xFF03),a	; bank 0, I/O on
	push	af
	ld	bc,#0xDF02
	out	(c),l
	inc	c
	out	(c),h
	inc	c
	xor	a
	out	(c),a
	inc	c
	out	(c),e
	inc	c
	out	(c),d
	inc	c
	ld	a,#1
	out	(c),a
	xor	a
	out	(c),a
	inc	c
	out	(c),a
	pop	af
	ld	c,#1
	out	(c),a
	ld	(0xFF01),a	; bank 0 I/O off
	ret

_reu_probe:
	call	map_io
	ld	bc,#0xDF00
	in	a,(c)
	and	#0X07
	ld	hl,#0
	jp	nz, map_kernel	; low bits not zero - not an REU
	ld	hl,#tag
	ld	de,#0
	ld	(hl),d
	call	_reu_putb	; We write 0 into the first REU byte
	ld	(hl),#0xAA
	call	_reu_getb
	ld	a,(hl)		; We scribbled over our tag byte and got it back
	or	a		; if this isn't back to 0 it's not an REU
	jr	nz, not_reu
	;	Walk the 16MB space in 64K chunks tagging and testing
_reu_test:
	inc	d
	ld	(hl),d
	call	_reu_putb
	call	_reu_getb
	ld	a,(hl)
	cp	d
	jr	nz, _reu_over	; Write failed - end
	push	de
	ld	d,#0
	call	_reu_getb
	pop	de
	ld	a,(hl)		; Write wrapped - end
	or	a
	jr	nz, _reu_over
	inc	d
	jr	nz, _reu_test	; Test all pages
	ld	hl,#32768	; 16MB - fully loaded
	ret
_reu_over:
	; D is the number of 64K banks (128 buffers per bank)
	xor	a
	srl	d
	rra
	ld	h,d
	ld	l,e
	jp	map_kernel
not_reu:
not_geo:
	ld	hl,#0
	jp	map_kernel

tag:	.byte	0

	.globl	_geo_probe

geopat:
	ld	bc,#0xDFFF
	call	iowrite8
	dec	c
	xor	a
	call	iowrite8
	; Selected block 0
	ld	bc,#0xDE00
	jp	iowrite16


_geo_probe:
	; Georam is a simple banked paged RAM at $DExx controlled by $DFFx
	; The probe isn't quite as simple as we would like because the board
	; may wrap or fail writes to absent blocks
	ld	hl,#0xDE00		; DE address E also 0
	ld	de,#0xDFFF		; Bank
	call	map_io
	xor	a
	ld	(0xDFFE),a		; Page 0 of 16K bank
	; Do a quick RAM sanity check
	ld	a,(hl)			; Grab what is there
	inc	hl
	cpl
	ld	(hl),a			; Write complement a byte on
	cpl
	dec	hl
	cp	(hl)			; Check the original matches
	ld	a,#0			; Preserves flags
	jr	nz, geo_wrap
geo_scan:
	ld	(de),a			; Bank select
	cpl
	ld	(hl),a			; Mark with own code inverted
	cp	(hl)			; Check it wrote
	jr	nz, geo_wrap_c		; Off end of available memory
	cpl
	ld	(hl),a			; Mark with own code non inverted
	cp	(hl)			; Check it wrote
	jr	nz, geo_wrap		; Off end of available memory
	ld	c,a			; Save bank number
	xor	a
	ld	(de),a			; Bank 0
	ld	a,(hl)			; Check didn't wrap into bank 0
	or	a			; should still be zero
	ld	a,c			; Get bank number back into A
	jr	nz, geo_wrap
	inc	a
	jr	nz, geo_scan		; Max 256 blocks
	ld	hl,#8192		; 8192 blocks
	jp	map_kernel
geo_wrap_c:
	cpl
geo_wrap:
	; We found A 16K banks
	ld	h,#0
	ld	l,a
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	jp	map_kernel
