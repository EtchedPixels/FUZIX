;
;	Interface glue for the beta disk.
;
;	This is a work in progress. The betadisk is only accessible when the
;	built in ROM is mapped. It unmaps itself when we return to RAM. It
;	has a sort of "BIOS" type ABI but that assumes we are in ZX Basic
;	and also screws around with memory behind the caller.
;
;	So we have to do things differently. Instead we treat the TR-DOS 5
;	ROM as a ROP exploit target.
;
;	This is a WIP and assumes the TR-DOS v5 ROM
;

		.module betadisk

		.globl _betadev, _betaaddr, _betasector, _betatrack
		.globl _betauser, _betacount

		.globl _trdos_init, _trdos_read, _trdos_write

		.globl map_process_save, map_kernel_restore

		.globl outhl

		.area _COMMONDATA
;
;	Ordering matters as we load some as pairs
;
_betadev:	.db	0		; disk number
_betaaddr:	.dw 	0		; buffer ptr
_betasector:	.db	0		; sector (1 based)
_betatrack:	.db	0		; track (low bit is side)
_betauser:	.db	0		; paging info
_betacount:	.db	0		; count of blocks
_betacmd:	.db	0		; command byte
_betatrackreg:	.db	0		; used to track h/w
saved_sp:	.dw	0		; stack for unwinds
saved_hl:	.dw	0		; HL for trdos entry

		.area _COMMONMEM

_trdos_read:
		ld a, #0x80
		ld (_betacmd), a
trdos_op:
		ld bc, (_betasector)
		call seekhead
		jr nz, error
		ld a, (_betauser)
		or a
		push af
		call z, map_process_save
		call trdos_doit
		pop af
		call z, map_kernel_restore
		ld h, b
		ld c, l
		ret
error:
		ld hl, #-1
		ret

_trdos_write:
		ld a, #0xA0
		ld (_betacmd), a
		jr trdos_op


;
;	Head is on the right track, density is right, drive is right
;
trdos_doit:
		ld hl, (_betaaddr)
		ld a, (_betacmd)
		cp #0x80
		jr nz, dowrite
		call write_cmd_delay
		call wait_drq_and_read
		call read_status
		ret
dowrite:	call write_cmd_delay
		call wait_drq_and_write
		call read_status
		; Check status
		ret


;
;	Hook into the ROM to issue a reset and seek to 0
;
trdos_reset:
		; Set the frame up for the exception path
		push bc
		push de
		ld (saved_sp), sp
		ld hl, #0x3d98
		call trdos_call
		xor a
		; We can't easily read the real register so shadow it
		ld (_betatrackreg), a
		pop de
		pop bc
		ret

;
;	Drive head and seek management. Assumes track register is
;	correctly loaded
;
seekhead:
		ld a, (_betadev)
		or #0x24		; MFM, don't reset
		ld b, a
		ld a, (_betatrack)
		or a
		rra
		jr c, topside
		; bottom
		set 3,b
topside:
		ld (_betatrack), a
		ld a, b
		call trdos_outsys	; Select the drive/side

		; need delays on the various side/disk switches
		; check if need to seek
		call trdos_outdata
;		call nap
		ld a, #0x18		; head load, 6ms
		ld hl, #0x2f57		; issue command, wait for INT
		call trdos_call
		call read_status
		; Check status - save new value
		ret
;
;	Arbitrary code execution in the TRDOS ROM
;
trdos_call:	push hl
		ld hl, (saved_hl)
		jp 0x3D2F

;
;	An entry point that writes A to the command register and returns
;
trdos_outctrl:	ld hl, #0x2fc3
		call trdos_call
		ret
;
;	An entry point that writes A into the track register and returns
;
trdos_outtrk:	push hl
		ld hl, #0x1e3a
		call trdos_call
		pop hl
		ret
;
;	Write A into the interface control register
;
trdos_outsys:	push hl
		ld hl,#0x1ff3
		call trdos_call
		pop hl
		ret
;
;	Write the other registers via 0x20B8
;
trdos_outsec:	push bc
		push de
		push hl
		ld bc, #0x015F
;
;	Write D into register C B times
;
trdos_outit:
		ld d, a
		ld hl, #0x20B8
		call trdos_call
		pop hl
		pop de
		pop bc
		ret

trdos_outdata:	push bc
		push de
		push hl
		ld bc, #0x017F
		jr trdos_outit

;
;	Low level I/O routines we can borrow
;

wait_drq_and_read:
		ld (saved_hl), hl
		ld hl, #0x3fd5
		call trdos_call
		ret

wait_drq_and_write:
		ld (saved_hl), hl
		ld hl, #0x3fba
		call trdos_call
		ret

write_cmd_delay:
		push hl
		ld hl, #0x2f57
		call trdos_call
		pop hl
		ret

;
;	This is a somewhat evil dive into the middle of some code which in
;	some cases will attempt to jump back into BASIC. We catch it trying
;	to do so on errors and longjmp out of it.
;
read_status:
		push bc
		push de
		ld (saved_sp), sp
		ld d, #1
		xor a
		ld (0x5d15), a
		dec a
		ld (0x5d0e), a
		ld hl, #0x3f33
		call trdos_call
unwind:
		; b now holds status
		ld l, b
		ld h, #0
		pop de
		pop bc
		ret

;
;	TRDOS tried to print an error message. Grab it as it tries to
;	go to the BASIC ROM and instead recover our stack and return an
;	error
;
_trdos_exception:
		pop hl
		ld a, l
		cp #0x54		; checking break key
		jr z, cbreak
		ld sp, (saved_sp)
		jr unwind
cbreak:
		scf			; no break (make timer based)
		ret


;
;	Interceptor for ROM
;
_trdos_init:
		ld a, #0xc3
		ld (0x5cc2), a
		ld hl, #_trdos_exception
		ld (0x5cc3), hl
		ret
