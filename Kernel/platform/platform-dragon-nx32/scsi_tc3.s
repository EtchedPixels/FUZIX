	.module scsi_tcs

	include "kernel.def"
	include "../../cpu-6809/kernel09.def"

	.globl _si_read
	.globl _si_write
	.globl _si_writecmd
	.globl _si_select
	.globl _si_clear
	.globl _si_reset
	.globl _si_deselect

	.area .common

;
;	For the moment we only allow one scsi adapter compiled in. That
;	wants tidying up some day
;
;
;	FIXME: read/write need to honour user/kernel swap etc flags
;	here. For now this will only work for simple testing stuff
;
_si_read:			; X is pointer
	pshs y,u
	tfr s,u			; u holds our frame pointer to throw
	bsr waitreq		; an exception
	ldy #_si_dcb+16		; length word
	bita #0x08		; check CMD asserted
	bne si_busfailw
	lda $FF70
	sta ,x+
	leay -1,y
	bne _si_read
	clrb			; Return 0
	puls y,u,pc

_si_write:			; X is pointer, dcb block gives length
	pshs y,u
	tfr s,u			; u holds our frame pointer to throw
	bsr waitreq		; an exception
	ldy _si_dcb+16		; length word
	bita #0x08		; check CMD asserted
	bne si_busfailw
	lda ,x+
	sta $FF70
	leay -1,y
	bne _si_write
	clrb			; Return 0
	puls y,u,pc
timeout:
	tfr u,s			; Throw an exception
si_busfailw:
	ldb #-1
	puls y,u,pc

;
;	Must preserve B and X
;
waitreq:
	pshs y
	ldy #30000		; Guess a timeout !
waitreql:
	leay -1,y
	beq timeout
	lda $FF71		; errors back
	bita #0x01
	bne waitreql
	rts

	.area .text

mpi_scsi:
	ldb #_cartslots
	cmpb #1
	beq si_rts
	ldb #_scsi_slot
	jmp _mpi_set_slot
	
_si_writecmd:			; DCB, B is len (check B or X)
	pshs y,u
	tfr s,u			; Required exception frame for waitreq
_si_writecmdl:
	jsr waitreq
	ldx #_si_dcb
	bita #0x08		; check CMD asserted
	beq si_busfail
	lda ,x+
	sta $FF70
	decb
	bne _si_writecmdl
	clrb			; Return 0
	puls y,u,pc
si_busfail:
	ldb #-1			; Error (should sort some codes for type!)
	puls y,u,pc

_si_select:			; Select device
	bsr mpi_scsi		; Set the MPI
	ldb #_si_dcb+20		; Device number
	cmpb #7			; controller
	beq si_seltimeo		; FIXME: different errorcodes ? 
	ldx #0
	lda #1
	tstb			; Fast path the usual device 0 case
	beq si_dobsyw
si_shiftdev:
	rola
	decb
	bne si_shiftdev
si_dobsyw:
	ora #0x80		; We mus also assert our own device id on
				; the bus (7)
si_bsyw:
	leax -1,x
	beq si_seltimeo
	lda $FF71		; FIXME: needs timeouts
	bita #0x02
	bne si_bsyw		; Wait for BSY to drop
	stb $FF70		; Put B on the data lines
	stb $FF71		; trigger a select
si_bsyc:
	leax -1,x
	beq si_seltimeo
	lda $FF71
	bita #0x02
	beq si_bsyc		; The device asserts BSY for us
	clrb			; All good
	rts
si_seltimeo:
	ldb #-1
_si_reset:
si_rts:
	rts

_si_clear:
	bsr mpi_scsi
	clr $FF70
	; Fall through
_si_deselect:
	ldb si_savedslot
	cmpb #255
	beq si_rts
	jmp _mpi_set_slot

	.area .data

si_savedslot
	.db 255			; No restore (eg no MPI)
