	; exports
	.export _discard_size

	; startup code

	.code

;
;	We get booted from the CP/M boot ROM off floppy A:
;	We are loaded at 0x0100 flat with the high things packed
;
	.ascii "NCB"			; The boot ROM expects this magic
start:
	ld	a,1			; sector
	ld	(0xFC76),a
	ld	hl,0x0300		; We are in 0100-02FF
	ld	(0xFC82),hl
	ld	a,0x71			; Sector count (512 bytes per sector)
	ld	(0xFC18),a
next:
	call	0xF003			; Boot helper
	ld	hl,0xFD0A
	ld	de,(0xFC82)
	ld	bc,512
	ldir
	ld	(0xFC82),de
	ld	hl,0xFC76
	inc	(hl)
	ld	a,10
	cp	(hl)
	jr	nz, same
	ld	(hl),0
	dec	hl
	inc	(hl)			; Next track (single sided)
same:
	ld	hl,0xFC18
	dec	(hl)
	jr	nz, next
	; The kernel is now loaded
	;
	; The boot block loaded 0100-02FF
	; We loaded 0300-E500, and we have the commondata for FC00
	; loaded at E000-E3FF for now
	;
	ld	sp, kstack_top
	; move the common memory where it belongs
	ld	hl,0xE000
	ld	de, __commondata
	ld	bc, __commondata_size
	ldir

	; then zero the BSS area
	ld	hl, __bss
	ld	de, __bss + 1
	ld	bc, __bss_size - 1
	ld	(hl), 0
	ldir
	; Zero buffers area
	ld	hl, __buffers
	ld	de, __buffers + 1
	ld	bc, __buffers_size - 1
	ld	(hl), 0
	ldir
	ld	hl, __common
	ld	de, __discard
	or	a
	sbc	hl,de
	ld	(_discard_size),hl
	call	init_early
	call	init_hardware
	call	_fuzix_main
	di
stop:	halt
	jr	stop

	.discard

_discard_size:
	.word 0
