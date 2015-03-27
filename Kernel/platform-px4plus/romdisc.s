;
;	ROM drive interfaces
;
	.globl _rom_sidecar_read
	.globl _rom_cartridge_read

	.globl _romd_mode
	.globl _romd_addr
	.globl _romd_size
	.globl _romd_off

	.globl map_process_save
	.globl map_kernel_restore

	.area _COMMONMEM

_rom_sidecar_read:
	ld a, (_romd_mode)
	or a
	push af
	call nz, map_process_save
	xor a
	out (0x90), a		; low byte is always zero
	ld hl, #_romd_off
	ld de, (_romd_size)	; Always multiple of 512 - don't need low FIXME
rom_sc_loop:
	ld a, (hl)
	out (0x91), a		; mid byte
	inc hl
	ld a, (hl)
	out (0x92), a		; top
	ld bc, #0x93		; port 93, 256 repeats
	ld hl, (_romd_addr)
	inir			; copy 256
	dec d
	jr z, rom_sc_done
	ld (_romd_addr), hl	; save pointer
	ld hl, #_romd_off
	inc (hl)		; next page
	jr rom_sc_loop
rom_sc_done:
	pop af
	ret z
	jp map_kernel_restore


_rom_cartridge_read:
	ld a, (_romd_mode)
	or a
rom_cr1:
	push af
	call nz, map_process_save
	xor a
	out (0x10), a		; low byte is always zero
	ld hl, #_romd_off
	ld de, (_romd_size)	; Always multiple of 512 - don't need low FIXME
rom_ca_loop:
	ld a, (hl)
	out (0x11), a		; mid byte
	ld bc, #0x10		; port 10, 256 repeats
	ld hl, (_romd_addr)
rom_ca_cp:
	out (c), b		; set low byte
	in a, (0x12)		; fetch from ROM
	ld (hl), a		; save
	inc hl
	inc b
	jr nz, rom_ca_cp	; For the 256 byte block
	dec d
	jr z, rom_ca_done
	ld (_romd_addr), hl	; save pointer
	ld hl, #_romd_off
	inc (hl)		; next page
	jr rom_ca_loop
rom_ca_done:
	pop af
	ret z
	jp map_kernel_restore

