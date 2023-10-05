	;
	; common Dragon platform
	;

	.module dragon

	; exported
	.globl _mpi_present
	.globl _mpi_set_slot
	.globl _cart_hash
	.globl _cart_analyze_hdb
	.globl _hdb_offset
	.globl _hdb_id
	.globl _hdb_port
	.globl _hdb_timeout
	.globl _bufpool
	.globl _discard_size

	; imported
	.globl unix_syscall_entry
	.globl fd_nmi_handler
	.globl size_ram
	.globl null_handler
	.globl _vid256x192
	.globl _vtoutput

	; exported debugging tools
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	include "kernel.def"
	include "../../cpu-6809/kernel09.def"


	.area .vectors
	;
	;	At 0x100 as required by the Dragon ROM
	;
	jmp badswi_handler			; 0x100
	jmp badswi_handler			; 0x103
	jmp unix_syscall_entry 		; 0x106
	jmp fd_nmi_handler			; 0x109
	jmp interrupt_handler		; 0x10C
	jmp firq_handler			; 0x10F

	.area	.buffers
	;
	;	We use the linker to place these just below
	;	the discard area
	;
_bufpool:
	.ds	BUFSIZE*NBUFS

	;	And expose the discard buffer count to C - but in discard
	;	so we can blow it away and precomputed at link time
	.area	.discard
_discard_size:
	.db	__sectionlen_.discard__/BUFSIZE

init_early:
	ldx #null_handler
	stx 1
	lda #0x7E
	sta 0
	sta 0x0071		; BASIC cold boot flag
	rts

init_hardware:
	jsr size_ram
	; Turn on PIA  CB1 (50Hz interrupt)
	lda 0xFF03
	ora #1
	sta 0xFF03
	jsr _vid256x192
	jsr _vtinit
	; NTSC or PAL/SECAM ?
	ldx	#0
	lda	$ff02
waitvb0
	lda	$ff03
	bpl	waitvb0		; wait for vsync
	lda	$ff02
waitvb2:
	leax	1,x		; time until vsync starts
	lda	$ff03
	bpl	waitvb2
	stx	_framedet
	rts

	.globl _framedet

_framedet:
	.word	0

; old p6809.s stuff below

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl _need_resched


            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl unix_syscall_entry
	    .globl fd_nmi_handler

            .area .common

_plt_reboot:
	    orcc #0x10
	    IFDEF MOOH
	    clr 0xFF90		; disable MMU on MOOH
	    ELSE
	    clr 0xFFBE		; disable nx32 memory
	    ENDC
	    clr 0x0071
	    jmp [0xFFFE]

_plt_monitor:
	    orcc #0x10
	    bra _plt_monitor

___hard_di:
	    tfr cc,b		; return the old irq state
	    orcc #0x10
	    rts
___hard_ei:
	    andcc #0xef
	    rts

___hard_irqrestore:		; B holds the data
	    tfr b,cc
	    rts

            .area .text


;
;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW


            .area .common

;
;	In the Dragon nx32 case our vectors live in a fixed block
; 	and is not banked out.
;
_program_vectors:
	rts

;
;	Helpers for the MPI and Cartridge Detect
;

;
;	oldslot = mpi_set_slot(uint8_t newslot)
;
_mpi_set_slot:
	tfr b,a
	ldb 0xff7f
	sta 0xff7f
	rts
;
;	int8_t mpi_present(void)
;
_mpi_present:
	lda 0xff7f	; Save bits
	tfr a,b
	lsrb
	lsrb
	lsrb
	lsrb
	eorb 0xff7f
	andb #0x03	; We expect to see the bits 5-4 and 1-0 matching
	bne nompi	; not guaranteed but a good rule of thumb for us
	ldb #0xff	; Will get back 33 from an MPI cartridge
	stb 0xff7f	; if the emulator is right on this
	ldb 0xff7f
	andb #0x33
	cmpb #0x33
	bne nompi
	clr 0xff7f	; Switch to slot 0
	ldb 0xff7f
	andb #0x33	; We can't trust the high bits
	bne nompi
	incb
	sta 0xff7f	; Our becker port for debug will be on the default
			; slot so put it back for now
	rts		; B = 0
nompi:	ldb #0
	sta 0xff7f	; Restore bits just in case
	rts

;
;	uint16_t cart_hash(void)
;
_cart_hash:
	pshs cc
	orcc #0x10
	ldx #0xC000
	ldd #0
	clr $FFBE	; Map cartridge
hashl:
	addd ,x++
	cmpx #0xC200
	bne hashl
	tfr d,x
	clr $FFBF	; Return to normality
	puls cc,pc

_cart_analyze_hdb:
	pshs cc
	orcc #0x10
	clr $FFBE
	ldd 0xD93B	; I/O port
	std _hdb_port
	ldd 0xD93D	; Timeout and ID if SCSI
	std _hdb_timeout
	; Shortly after that fixed block we will find the sign on and
	; copyright, which tell us what interface we are for.
	ldx #0xD940
hdb_s_next:
	cmpx #0xE000
	beq no_sign	; No sign of the sign on !
	lda ,x+
	cmpa #'H'
	bne hdb_s_next
	lda ,x+
	cmpa #'D'	; This is safe as we know the bytes before
	bne hdb_s_next	; our match are AUTOEXEC.BAS so won't partially
	lda ,x+		; match !
	cmpa #'B'
	bne hdb_s_next
	lda ,x+
	cmpa #'-'
	bne hdb_s_next
	;
	; We have found the HDB-. X now points at the D of DOS
	;
	leax 6,x	; Skip version
	;
	; We now have to find a space
	;
hdb_s_spc:
	cmpx #0xE000
	beq no_sign
	lda ,x+
	cmpa #0x20
	bne hdb_s_spc
	;
	; This is followed by a string. For those we care about the first
	; letters are sufficient to tell them apart
	;
	; LB : IDE LBA
	; ID : IDE CHS
	; TC : TC^3 SCSI
	; KE : Kenton
	; LR : LR Tech 
	; HD : HD-II
	; 4- : 4-N-1
	; DW : Drivewire stuff	} Need more identification but are not 
	; BE : Becker ports 	} interesting to us anyway
	; J& : J&M CP
	ldx ,x
ret1:
	clr $FFBF
	puls cc,pc
no_sign:
	ldx #-1
	bra ret1

; Need to be here so they can be written with cart paged in
_hdb_port:
	.dw 0
_hdb_timeout:
	.db 0
_hdb_id:
	.db 0
_hdb_type:
	.db 0	

	.area .text
;
;	Joystick helper
;
;	jsread(buffer)
;
;	Returns a buffer of words in the format
;	right left/right, button
;	right up/down, button
;	left left/right, button
;	left up/down, button
;
	.globl _jsread

_jsread:
	; Buffer is in X on entry
	pshs u
	lda #$FF
	sta $FF02		; Keyboard scan lines off
	lda #$08		; Select right joystick
	sta $FF23		; Sound off a moment
	bsr jstwo
	lda #$09
	bsr jstwo
	puls u
	rts
jstwo:
	sta $FF03		; P0 CR B - select joystick L or R
	lda #$04
	sta $FF01		; X
	bsr jsfind
	lda #$0C		; Y
	sta $FF01
	; Fall through
jsfind:
	ldu #jstmp
	; Binary search the joystick DAC position
	lda #$20
	sta ,u		; start in the middle and binary search
jssearch:
	lsr ,u
	beq jsdone
	sta $FF20
	tst $FF20
	bpl jsover
	adda ,u
	bra jssearch
jsover:
	suba ,u
	bra jssearch
jsdone:
	ldb $FF20	; save fire button in bit 0
	std ,x++
	rts

	.area .data
jstmp:
	.byte 0

	.area .common
;
;	FIXME:
;
firq_handler:
badswi_handler:
	rti

;
;	debug via printer port
;
outchar:
	sta 0xFF02
	lda 0xFF20
	ora #0x02
	sta 0xFF20
	anda #0xFD
	sta 0xFF20
	rts

	.area .commondata

_need_resched: .db 0
