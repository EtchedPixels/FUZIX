;
;	Target specific support
;
#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

	.export _lobo_io

	.abs
	.org 0x0000
_lobo_io:

	.buffers

	.export _bufpool

_bufpool:
	.ds BUFSIZE * NBUFS;

	.commondata

	.export _int_disabled

_int_disabled:
	.byte 1

	.export _sio_r
_sio_r:
	.byte	0x03
	.byte	0xC1
	.byte	0x04
	.byte	0xC4
	.byte	0x05
	.byte	0xEA

	.code

	.export _sio2_out

_sio2_out:
	push	bc
	ld	b,0x06
	ld	c,l
	ld	hl,_sio_r
	otir
	pop	bc
	ret

	.common

	.export plt_interrupt_all
	.export _program_vectors

plt_interrupt_all:
_program_vectors:
	ret

	.export	_plt_monitor
_plt_monitor:
	di
	halt
	jr	_plt_monitor

	.export _plt_reboot
_plt_reboot:
	di
	xor	a
	ld	(0x07DC),a
	; I/O low with ROM enabled
	rst	0

	.code

	.export init_early
init_early:
	ret

	.export _plt_idle
_plt_idle:
	halt
	ret

	.discard

	.export init_hardware

init_hardware:
	; Select font
	ld	a,0x04
	ld	(0x07DC),a
	ld	hl,_fontdata_8x8
	ld	de,0x0100		; Character 32
	ld	bc,0x0300		; 96 characters
	ldir
	ld	a,0x02
	ld	(0x07DC),a		; VRAM back
	; Wipe the display
	ld	hl,0x0000
	ld	de,0x0001
	ld	bc,0x03FF
	ld	(hl),0x20
	ldir
	ld	hl,0x0C00
	ld	de,0x0C01
	ld	bc,0x03FF
	ld	(hl),0x20
	ldir
	; Now bring up the video
	ld	de,crtbits
	ld	hl, 0x07E0		; CRT reg
	ld	bc,0x1000
crtloop:
	ld	(hl),c		; Register
	ld	a,(de)
	inc 	de
	ld	(0x07E1),a
	inc	c
	djnz	crtloop

	ld	hl,128
	ld	(_ramsize),hl
	ld	l,64
	ld	(_procmem),hl
	;	Interrupts with IM2 jump to last 2 bytes of any page we want
	ld	a,0x06		; Top of RAM400
	ld	i,a
	ld	hl,interrupt_handler
	ld	(0x06FE),hl
	im	2
	ret

crtbits:
	; Standard 80x24 set up
	.byte	119
	.byte	80
	.byte	97
	.byte	6
	.byte	27
	.byte	12
	.byte	24
	.byte	26
	.byte	0
	.byte	8
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0

	.commondata

	.export _kernel_flag

mapsave:
	.byte	0
_kernel_flag:
	.byte	1

	.export map_buffers
	.export map_kernel_restore
	.export map_kernel_di
	.export map_kernel
	.export map_proc
	.export map_proc_hl
	.export map_for_swap
	.export map_proc_a
	.export map_save_kernel
	.export map_restore
	.export map_proc_always
	.export map_proc_always_di

map_buffers:
map_kernel_restore:
map_kernel_di:
map_kernel:	
	push	af
	ld	a,0x40
	ld	(0x07FC),a
	pop	af
	ret

map_proc:
	ld	a,h
	or	l
	jr	z, map_kernel
map_proc_hl:
	ld	a,(hl)
map_for_swap:
map_proc_a:
	ld	(0x07FC),a
	ret

map_save_kernel:
	push	af
	ld	a,(0x07FC)
	ld	(mapsave),a
	ld	a,0x40
	ld	(0x07FC),a
	pop	af
	ret

map_restore:
	push	af
	ld	a,(mapsave)
	ld	(0x07FC),a
	pop	af
	ret

map_proc_always:
map_proc_always_di:
	push	af
	ld	a,(_udata + U_DATA__U_PAGE)
	ld	(0x07FC),a
	pop	af
	ret

	.export outchar

outchar:	; SIO debug out
	push	hl
	ld	hl,0x07E5
wait:
	bit	2,(hl)
	jr	z, wait
	dec	hl
	ld	(hl),a
	pop	hl
	ret

;
;	SASI block helpers
;

sasi_map:
	ld	bc,(_sasi_len)
	ld	de,0x07F1		; Self acking data
	ld	a,(_td_raw)
	or	a
	ret	z
	dec	a
	jp	z,map_proc_always
	; Must be swap
	ld	a,(_td_page)
	jp	map_for_swap


	.export _sasi_outblock
	.export _sasi_inblock
	.export _sasi_outbytes
	.export _sasi_inbytes

_sasi_outblock:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	sasi_map
	ld	b,0
outn:
	ld	a,(hl)
	ld	(de),a
	inc	hl
	ld	a,(hl)
	ld	(de),a
	inc	hl
	djnz	outn
	pop	bc
	jp	map_kernel

_sasi_inblock:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	sasi_map
	ld	b,0
inn:
	ld	a,(de)
	ld	(hl),a
	inc	hl
	ld	a,(de)
	ld	(hl),a
	inc	hl
	djnz	inn
	pop	bc
	jp	map_kernel

_sasi_outbytes:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	sasi_map
outb:
	ld	a,(0x07F4)
	and	0x1C		; C/D I/O and BUSY
	cp	0x04
	jr	nz, bailo
	ld	a,(hl)
	ld	(de),a
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz, outb
bailo:
	pop	bc
	jp	map_kernel

_sasi_inbytes:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	sasi_map
inb:
	ld	a,(0x07F4)
	and	0x1C		; C/D I/O and BUSY
	cp	0x0C
	jr	nz, baili
	ld	a,(de)
	ld	(hl),a
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz, inb
baili:
	pop	bc
	jp	map_kernel

	.code
;
;	Helpers
;
	.export _memmove
_memmove:
	ld	hl,7
	add	hl,sp
	push	bc
	ld	b,(hl)
	dec	hl
	ld	c,(hl)
	ld	a,b
	or	c
	jr	z, move0
	dec	hl
	ld	d,(hl)
	dec	hl
	ld	e,(hl)
	dec	hl
	ld	a,(hl)
	dec	hl
	ld	l,(hl)
	ld	h,a
	ex	de,hl
	; Copy from HL to DE for BC bytes taking care of direction
	push	hl
	or	a
	sbc	hl,de
	pop	hl
	jr	c, copy_back
	ldir
move0:
	pop	bc
	ret
copy_back:
	dec	bc
	add	hl,bc
	ex	de,hl
	add	hl,bc
	ex	de,hl
	lddr
	ldd
	pop	bc
	ret
