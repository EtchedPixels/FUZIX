;;;
;;;  A Fuzix booter for the CoCo3
;;;

;;; This bootloader works from a DECB-like evironment, It loads
;;; FUZIX.BIN from a DECB disk and plops it in memory, starting
;;; at physical address 0x0000.
	org	$7a00		; where am I loaded.

frame	.dw	0		; on entry frame pointer
npage	.db	0		; next page no.
pos	.dw	0		; buffer pos in memory
tickp	.dw	$400+(32*15)	; ticker next position
tickb	.db	0		;
size	.dw	0		; no of grans
secs	.dw	0		; init value of of sectors for each screen block
scount	.dw	0		; sector counter
nampre	fcn	/"FUZIX.BIN/	; " image to load
	rmb	6		; pad for max filename
wbuf	rmb	32		; word buffer for cmdline parsing
wptr	.dw	$88		; static data for word routine
bootstr	fcn	"BOOT="
fnf	.db	13
	fcc	"KERNEL FILE NOT FOUND"
	.db	13
	.db	0

	;; And the Kick-off
start
	sts	frame
	lda	#13
	jsr	0xa282
	ldd	#$400+(32*14)
	std	$88
	lda	#'F		; print F
	jsr	0xa282		;
	;; Move to task one
	ldx	#$ffa0
	ldu	#$ffa8
	ldd	,x++		; copy mmu regs
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldd	,x++
	std	,u++
	ldb	#1
	stb	$ff91		; set mmu to task 1
	;; move and process command line
	jsr	cpcmd
	jsr	bootfile
	;; open kernel image file
	ldb	#'I		; input mode
	jsr	open
	lbcs	abort
	lda	#'Z		; print "Z"
	jsr	$a282
	;; calculate sectors for each screen block
	ldd	size
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	std	secs
	std	scount
	;; Load BIN file
	;;
	ldb	$6f		; get current file no.
	pshs	b		; save on stack
	ldb	#1
	stb	$6f		; switch in/out routine to disk file #1
c@	jsr	$a176		; get a byte in A
	cmpa	#$ff		; compare A to ff
	beq	post		; jump to post able handling
	;; preamble
	jsr	getw		; D = length address
	tfr	d,y		; U = length
	jsr	getw		; D = load address
	jsr	setload		; set load address
d@	jsr	$a176		; A = byte
	jsr	tick
	jsr	putb		; put into kernel memory
	leay	-1,y		; decrement U
	bne	d@		; loop
	bra	c@		; try next byte in stream
	;; postable
post	jsr	getw		; get zero's
	cmpd	#0		; test D
	lbne	abort		; abort if not zero
	jsr	getw		; get exec address
	pshs	d		; save on stack
	jsr	close		; close DECB file
	;; report load
	clr	$6f		; set stream to console
	lda	#'L
	jsr	$A282
	ldd	#0		; Address = 0
	jsr	setload
	ldy	#bounce_end-bounce
	ldx	#bounce
e@	lda	,x+
	jsr	putb
	leay	-1,y
	bne	e@
	lda	,s+		; get jmp address high byte
	jsr	putb		; put at end of bounce routine
	lda	,s+
	jsr	putb		; put at end of bounce routine
	lda	#'O
	jsr	$a282		; report load
	;; map in kernel block 0
	orcc	#$50		; turn off interrupts
	clr	$ffa8		; put bounce routine in memory
	jmp	$0		; and jump to bounce routine


;;; copy cmdline down to kernel 0
cpcmd	;; find command line in input buffer
	ldd	#$88		; set destination of cpy in kernel memory
	jsr	setload		;
	ldx	$a6		; X = position in program
a@	lda	,x+		; get a byte
	beq	c@		; end of line w/o remark token?
	cmpa	#$83		; is a ' token?
	beq	b@
	cmpa	#$82		; is a REM token?
	beq	b@
	bra	a@
b@	lda	,x+		; get one byte
c@	jsr	putb		; put it in kernel memory
	tsta
	bne	b@		; repeat if not done
	rts			; return

;;; parse cmdline for a word
word	pshs	d,x,u
	clra
	pshs	cc		; return C=clr by default,x,u
	lda	$ffa8
	pshs	a
	clr	$ffa8	      ; x ptr is kernel memory
	ldx	wptr		; X is src
	ldu	#wbuf		; u is dest
	;; remove leading spaces
a@	lda	,x+		; get a byte
	beq	d@		; if zero then no more to parse
	cmpa	#0x20		; is a space?
	beq	a@		; yes then repeat
	;; append non white space to word buffer
b@	sta	,u+		; append to word buffer
	lda	,x+		; get next word
	beq	c@		; if zero then end
	cmpa	#0x20		; is space?
	bne	b@		; no then repat
	;; end of line
c@	leax	-1,x
	stx	wptr		; save src ptr
	sta	,u+
	puls	a
	sta	$ffa8
	puls	cc,d,x,u,pc
	;; no more input
d@	coma
	tfr	cc,a
	sta	1,s
	bra	c@
	
;;; look for boot file name in cmd line
bootfile
	pshs	d,x,u
a@	jsr	word
	bcs	out@		; quit if no more words
	ldb	#5		; compare 5 chars
	ldx	#bootstr	;
	ldu	#wbuf		; 
b@	lda	,x+		; get string 1
	cmpa	,u+		; compare to string 2
	bne	a@		; get another word if not equal
	decb
	bne	b@
	;; ok we've found a "BOOT=": copy arg to filename buffer
	ldx	#nampre+1
c@	lda	,u+		; get a byte
	sta	,x+		; store a byte in buffer
	bne	c@		; repeat if not zero
	;; out of cmd to parse
out@	puls	d,x,u,pc

	
;;; This routine is copied down to kernel map 0
bounce
	;; map in rest of kernel
	ldx	#$ffa9
	lda	#1
a@	sta	,x+
	inca
	cmpa	#8
	bne	a@
	.db	$7e		; jump
bounce_end


;;; Gets next word from file
;;;    takes: nothing
;;;    returns: D = next word
getw
	jsr	$a176		; A = high byte
	tfr	a,b		; B = high byte
	jsr	$a176		; A = low byte
	exg	a,b		; flip D = next word
	jsr	tick
	jsr	tick
	rts


;;; Sets load address
;;;   takes: D = Address
;;;   returns: X = cpu mapped address
;;;   mods: D
setload
	pshs	d
	;; find block number
	lsra
	lsra
	lsra
	lsra
	lsra			; A= blk no
	sta	$ffaa		; put in mmu
	puls	d
	anda	#$1f		; D = offset
	addd	#$4000		; mmu offset
	std	pos		; store
	rts

;;; puts a byte into kernel memory, increments position
;;;   takes: A = byte to store
;;;   returns: nothing
;;;   mods:
putb	pshs	d,x
a@	ldx	pos
	cmpx	#$6000		; too far ?
	beq	inc@
	sta	,x+
	stx	pos
	puls	d,x,pc
inc@	ldx	#$4000		; inc pos
	stx	pos
	inc	$ffaa
	bra	a@

;;; Abort!
abort
	ldx	#fnf-1
	jsr	$b99c
	lds	frame
	rts


;;; Open a file
;;;   takes: B=ascii mode (I,O,D)
;;;   returns: C set on error
open	pshs	b		; save mode
	;; move local filename into BASIC's vars
	ldx	$a6
	pshs	x
	ldx	#nampre		; pointer to our local filename
	stx	$a6		; set CHARAD
	jsr	$c935		; have BASIC set up DNAMBF for us
	puls	x
	stx	$a6
	;; end of string copy - get directory info
f@	jsr	$c68c		; search directory, U=ram directory image
	tst	$973		; found?
	bne	g@		; yes - then return
	;; not found
	ldb	,s		; get mode
	cmpb	#'I		; is mode I?
	beq	err@		; yes then error!
	ldd	#$00ff		; basic/ascii
	bra	h@
	;; get size of file in granuals
g@	pshs	d,x,u
	ldb	2,u		; B = first granuals of file
	jsr	$cd1e		; get no of granuals
	andb	#0xf		; B = sectors used in last granual
	pshs	b		; save on stack ( last )
	clr	,-s		; as 16 bit value
	deca			; A = whole granuals
	ldb	#9
	mul			; D = sectors
	addd	,s++		; D = sectors in file
	std	size		; save
	puls	d,x,u
	;; copy directory stuff
	ldd	,u		; D = type/ascii
	ldb	#$ff		; force ascii
h@	std	$957
	ldd	#$100		; record length
	std	$976
	lda	,s		; A = Mode (I,O,D)
	ldb	#1		; B = FCB #1
	jsr	$c48d		; open file
out@	clra			; clear C
	puls	b,pc		; return
err@	coma			; set C
	puls	b,pc		; return



;;; Close file buffer
close
	ldb	#1		; set file number to 1
	stb	$6f
	jsr	$a42d		; close file
	clr	$6f		; set dev to screen
	rts


;;; Tick the ticker
tick
	pshs	d,x
	inc	tickb		; inc byte counter
	bne	out@		; return if not 256 bytes
	;; decrement block counter
	ldd	scount
	subd	#1
	std	scount
	bne	out@		; leave if sector/block counter is not done
	;; else put new screen block
	ldd	secs
	std	scount
	ldb	#$af		; a blue block
	ldx	tickp		; get position
	stb	,x+		; print it to screen buffer
	stx	tickp		; and save position
out@	puls	d,x,pc

	end	start
