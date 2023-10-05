;
;	Basic KC87 support
;
        .module kc87

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_proc_save
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
        .ds	(BUFSIZE * 4) ; adjust NBUFS in config.h in line with this
;
;	So we can check for overflow
;
kernel_endmark:

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
	ld	hl,#64		; FIXME - 16K + 2 x ?
	ld	(_ramsize), hl
	ld	hl,#32
	ld	(_procmem), hl

	im	2		; set Z80 CPU interrupt mode 2
	ret

;=========================================================================
; Common Memory (mapped low below ROM)
;=========================================================================
        .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	di			; TODO
	halt

;=========================================================================

_int_disabled:
	.db	1

; install interrupt vectors
_program_vectors:
	; Steal the ROM timer (we should possibly chain this TODO)
	ld	hl,#interrupt_handler
	ld	(0x0206),hl
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
	ld	a,h
	or	l				; HL == 0?
	jr	z,map_kernel			; HL == 0 - map the kernel

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
map_proc_save:
map_proc_always:
map_proc_always_di:
	push	af
	ld	a,(_rom_page)
	ld	(kmaprom),a
was_u:
	xor	a
	ld	(kmapped),a
	ld	(0xFC00),a	; ROM off if possible
	out	(5),a		; RAM second bank
	out	(7),a		; RAM on high
	pop	af
	ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map buffer pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
	; No work needed but remember for undo correctly
	push	af
	ld	a,(_rom_page)
	ld	(kmaprom),a
	pop	af
	ret

map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
	ld	a,#1
	ld	(kmapped),a
	ld	a,(kmaprom)
	ld	(_rom_page),a
	out	(0xFF),a	; set the ROM bank we were in before
	out	(4),a		; RAM first bank
	out	(6),a		; RAM off high
	ld	(0xF800),a	; ROM in
	pop	af
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push	af
	ld	a,(map_save)
	or	a
	jr	z, was_u
	; Not much work to do if we are restoring a kernel context as we
	; are currently have all but the megarom page right
	ld	a,(kmaprom)	; Restore the ROM contet
	ld	(_rom_page),a
	out	(0xFF),a
	pop	af
	ret
	

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push	af
	ld	a,(kmapped)
	ld	(map_save),a
	ld	a,(_rom_page)
	ld	(kmaprom),a	; Save former ROM context
	out	(4),a		; RAM first bank
	out	(6),a		; RAM off high
	ld	(0xF800),a	; ROM in
	pop	af
	ret

;
;	Helpers specific to the banked singe task support
;
	.globl map_save_kmap
	.globl map_restore_kmap

map_save_kmap:
	ld	a,(_rom_page)
	ret
map_restore_kmap:
	ld	(_rom_page),a
	out	(0xFF),a
	ret

kmapped:
	.byte	1
kmaprom:
	.byte	0
map_save:
	.byte 	0


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
	.globl __stub_3_2

_rom_page:
	.byte	0
callhl:
	jp	(hl)

;	Calling into a bank from common. We don't know the curent bank
;	but we need to restore accordingly. Initial hack assumes we are
;	in ROM banks 1-3 (not 0)
;
;	For the moment we ignore the issue of 8000-BFFF RAM needing to go
;	off on some cards to access rom at C000-E7FF. It's not fundamentally
;	a problem but we can debug it later. For now rely on the I/O 6/7
;	ports
;
__bank_0_1:
	ld	a,#1
	jr	bank0
__bank_0_2:
	ld	a,#2
	jr	bank0
__bank_0_3:
	ld	a,#3
	jr	bank0
__bank_0_4:
	ld	a,#4
bank0:
	pop	hl		; after call holds the true address
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl		; return is now right and DE is target
	ex	de,hl		; HL is now target in bank
	ld	bc,(_rom_page)	; ROM page into C
	ld	(_rom_page),a	; update with new page (must do before out)
	out	(0xFF),a	; switch to new bank
	ld	a, c		; old page
	dec	a		; bank 1 ?
	jr	z, retbank1
	dec	a		; bank 2 ?
	jr	z, retbank2
	dec	a
	jr	z, retbank3
	; Must be bank 4
	call	callhl		; 
	ld	a,#4
	ld	(_rom_page),a
	out	(0xFF),a	; Flip back
	ret
retbank1:
	call	callhl
	ld	a,#1
	ld	(_rom_page),a
	out	(0xFF),a	; Flip back
	ret
retbank2:
	call	callhl
	ld	a,#2
	ld	(_rom_page),a
	out	(0xFF),a	; Flip back
	ret
retbank3:
	call	callhl
	ld	a,#3
	ld	(_rom_page),a
	out	(0xFF),a	; Flip back
	ret

;
;	Fixed inter bank cases
;
__bank_1_2:
	ld	a,#0x02
outbank1:
	pop	hl		;	Retrieve target address
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl		;	Fixed up stack pointer
	ld	(_rom_page),a
	out	(0xFF),a	;	Switch banks
	ex	de,hl		;	targt into HL
	call	callhl
	ld	a,#1
	ld	(_rom_page),a
	out	(0xFF),a	;	switch bank to old bank
	ret
__bank_1_3:
	ld	a,#3
	jr	outbank1
__bank_1_4:
	ld	a,#4
	jr	outbank1

__bank_2_1:
	ld	a,#0x01
outbank2:
	pop	hl		;	Retrieve target address
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl		;	Fixed up stack pointer
	ld	(_rom_page),a
	out	(0xFF),a	;	Switch banks
	ex	de,hl		;	targt into HL
	call	callhl
	ld	a,#2
	ld	(_rom_page),a
	out	(0xFF),a	;	switch bank to old bank
	ret
__bank_2_3:
	ld	a,#3
	jr	outbank2
__bank_2_4:
	ld	a,#4
	jr	outbank2

__bank_3_1:
	ld	a,#0x01
outbank3:
	pop	hl		;	Retrieve target address
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl		;	Fixed up stack pointer
	ld	(_rom_page),a
	out	(0xFF),a	;	Switch banks
	ex	de,hl		;	targt into HL
	call	callhl
	ld	a,#3
	ld	(_rom_page),a
	out	(0xFF),a	;	switch bank to old bank
	ret
__bank_3_2:
	ld	a,#2
	jr	outbank3
__bank_3_4:
	ld	a,#4
	jr	outbank3

__bank_4_1:
	ld	a,#0x01
outbank4:
	pop	hl		;	Retrieve target address
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl		;	Fixed up stack pointer
	ld	(_rom_page),a
	out	(0xFF),a	;	Switch banks
	ex	de,hl		;	targt into HL
	call	callhl
	ld	a,#4
	ld	(_rom_page),a
	out	(0xFF),a	;	switch bank to old bank
	ret
__bank_4_2:
	ld	a,#2
	jr	outbank4
__bank_4_3:
	ld	a,#3
	jr	outbank4

;
;	Stubs. These are used when functions are called via pointers
;
__stub_0_1:
	ld	a, #1
	jr	__stub_in
__stub_3_2:
__stub_0_2:
	ld	a, #2
	jr	__stub_in
__stub_0_3:
	ld	a, #3
	jr	__stub_in
__stub_0_4:
	ld	a, #4
__stub_in:
	pop	hl
	ex	(sp),hl			; bank space is now return
	ex	de,hl			; HL is now target
	ld	bc,(_rom_page)		; old page into C
	ld	(_rom_page),a		; new page
	out	(0xFF),a
	dec	c			; old page 1 ?
	jr	z, __stub_ret_1
	dec	c
	jr	z, __stub_ret_2
	dec	c
	jr	z, __stub_ret_3
	; Nope so 4
	call	callhl			; call banked function
	ld	a,#4
__stub_ret:
	ld	(_rom_page),a
	out	(0xFF),a		; back to calling bank
	pop	bc			; Fix up stack
	push	bc
	push	bc
	ret
__stub_ret_1:
	call	callhl
	ld	a,#1
	jr	__stub_ret
__stub_ret_2:
	call	callhl
	ld	a,#2
	jr	__stub_ret
__stub_ret_3:
	call	callhl
	ld	a,#3
	jr	__stub_ret

	
;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
	push	hl
	ld	hl,(outpt)
	ld	(hl),a
	inc	hl
	ld	(outpt),hl
	pop	hl
	ret
outpt:	.word	0xEC00


	.area _COMMONMEM

;
;	Character pending or 0
;

	.globl _keycheck
;
_keycheck:
	push	ix
	push	iy
	call	0xF006
	or	a
	call	nz, 0xF009	
	pop	iy
	pop	ix
	ld	l,a
	ret

	.globl _rd_dptr
	.globl _rd_page
	.globl _rd_port
	.globl _rd_block

_rd_dptr:
	.word	0
_rd_page:
	.byte	0
_rd_port:
	.byte	0
_rd_block:
	.word	0
;
;	Output
;	C	data port for ramdisc
;	E	4 (ramdisc sectors per Fuzix sector)
;	HL	data pointer
;
;	B'	upper byte of block number on ramdisc (128 byte blocks)
;	C'	control port for ramdisc
;	L'	lower byte of block number on ramdisc
;
ramconf:
	ld	a,(_rd_port)	; get the port to use
	inc	a		; control port
	ld	c,a
	ld 	hl,(_rd_block)	; Requested 512 byte block
	add	hl,hl		; Turn HL into 128 byte sectors
	add	hl,hl		; as the disk is sector addressed
	ld	b,h
	out	(c),l		; Set the sector number
	ld	a,c
	exx			; BC' and L' for the block info
	ld	c,a
	dec	c		; Data port
	ld	hl,(_rd_dptr)	; Set up HL for the caller
	ld	e,#4
	ld	a,(_rd_page)	; check if we need to page user space in
	or	a
	jr	z, via_k
	jp	map_proc_always
via_k:				; kernel buffer
	jp	map_buffers

	.globl	_ramread

_ramread:
	call	ramconf
rrloop:
	ld	b,#0x7F
	inir
	ini
	exx
	inc	l
	out	(c),l
	exx
	dec	e
	jr	nz,rrloop
ramout:
	ld	(_rd_dptr),hl
	jp	map_kernel_restore

	.globl	_ramwrite

_ramwrite:
	call	ramconf
rwloop:
	ld	b,#0x80
	otir
	exx
	inc	l
	out	(c),l
	exx
	dec	e
	jr	nz,rwloop
	jr	ramout

