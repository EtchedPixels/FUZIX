;
;	Interface to a CP/M 2.2 BIOS
;
;	We assume the BIOS is in the common space.
;

	.module cpm

	.globl _cpm_const
	.globl _cpm_conin
	.globl _cpm_conout
	.globl _cpm_list
	.globl _cpm_punch
	.globl _cpm_reader
	.globl _cpm_home
	.globl _cpm_seldsk
	.globl _cpm_settrk
	.globl _cpm_setsec
	.globl _cpm_setdma
	.globl _cpm_read
	.globl _cpm_write
	.globl _cpm_listst
	.globl _cpm_sectran

	.globl _cpm_diskread
	.globl _cpm_diskwrite

	.globl _cpm_map

	.globl biosbase

	.globl map_proc_a
	.globl map_kernel

;
;	Call the bios function in the byte following, return to the layer
;	above.
;
; These calls also copy A into L on return but it doesn't matter

	.area _COMMONMEM

bioscall_bchl:
	ld b,h
bioscall_cl:
	ld c,l
bioscall_a:
	pop hl
	ld a,(hl)
	ld hl,(biosbase)
	ld l,a
	call callhl
	ld l,a
	ret
bioscall_hl:
bioscall:
	pop hl
	ld a,(hl)
	ld hl,(biosbase)
	ld l,a
callhl:	jp (hl)

_cpm_const:
	call bioscall_a
	.byte 6		; call 2
_cpm_conin:
	call bioscall_a
	.byte 9		; call 3
_cpm_conout:
	call bioscall_cl
	.byte 12	; call 4
_cpm_list:
	call bioscall_cl
	.byte 15	; call 5
_cpm_punch:
	call bioscall_cl
	.byte 18
_cpm_reader:
	call bioscall_a
	.byte 21
_cpm_home:
	call bioscall
	.byte 24
_cpm_settrk:
	call bioscall_bchl
	.byte 30
_cpm_setsec:
	call bioscall_bchl
	.byte 33
_cpm_setdma:
	call bioscall_bchl
	.byte 36
_cpm_read:
	call bioscall_a
	.byte 39
_cpm_write:
	call bioscall_cl
	.byte 42
_cpm_listst:
	call bioscall_a
	.byte 45
;
;	Special cases - sectran has two inputs, both return in HL
;
_cpm_seldsk:
	ld c,l
	ld e,h		; 0000 or FFFF
	ld d,h
	call bioscall_hl
	.byte 27	; call 9	DPH in HL
_cpm_sectran:		; can't be fastcall
	pop hl		; return address
	pop bc		; sector
	pop de		; table
	push de		; put them back
	push bc
	push hl
	call bioscall_hl
	.byte 48

;
;	CP/M 3 extensions (not something we can use yet due to the SCB)
;
_cpm_conost:
	call bioscall_a
	.byte 51
_cpm_auxist:
	call bioscall_a
	.byte 54
_cpm_auxost:
	call bioscall_a
	.byte 57
_cpm_devtbl:
	call bioscall_hl
	.byte 60
_cpm_devini:
	call bioscall_cl
	.byte 63
_cpm_drvtbl:
	call bioscall_hl
	.byte 66
_cpm_multio:
	call bioscall_hl
	.byte 69
_cpm_flush:
	call bioscall_hl
	.byte 72
; MOVE is useless to us
_cpm_time:
	call bioscall_cl
	.byte 75
; SELMEM and SETBNK won't be present in an unbanked BIOS nor XMOVE

;
;	Common space helpers of our own
;
_cpm_diskread:
	ld a,(_cpm_map)
	or a
	push af
	call nz, map_proc_a
	call _cpm_read
	; Return is in L
	pop af
	ret z
	jp map_kernel
_cpm_diskwrite:
	; This needs to preserve H into the cpm_write call
	ld a,(_cpm_map)
	or a
	push af
	call nz, map_proc_a
	call _cpm_write
	; Return is in L
	pop af
	ret z
	jp map_kernel

	.area _COMMONDATA

biosbase:
	.word 0			; updated at boot
_cpm_map:
	.byte 0
