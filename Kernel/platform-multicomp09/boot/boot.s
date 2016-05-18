;;;
;;; A Fuzix booter for the multicomp09 SDcard controller.
;;;
;;; Neal Crook April 2016
;;; This code started as a frankensteinian fusion of Brett's Coco3
;;; booter and my FLEX bootstrap loader.
;;;
;;; The booter is contained within a single 512-byte sector and is
;;; formatted like a standard MSDOS master boot record (MBR) -- except
;;; that the code is 6809 and not position-independent.
;;;
;;; An MBR is intended to go in sector 0 but I cannot easily arrange
;;; that on my SD and so I have placed it elsewhere -- and added an
;;; option to the FUZIX MBR parser to look for it at an arbitrary
;;; location. The partition data in the MBR is still absolute, though;
;;; *not* relative to the location of the MBR.
;;;
;;; The booter (the whole MBR) can live anywhere on the SD. It is loaded
;;; to 0xd000 and entered from there -- the load address is chosen simply
;;; to avoid the kernel; it may change in future if I adjust the memory
;;; map.
;;;
;;; The booter is completely self-contained within a 512-byte footprint:
;;; it uses no disk buffer and its small stack is allocated within its
;;; memory footprint.
;;;
;;; Environment: at entry, the multicomp ROM is disabled and the
;;; MMU is enabled and is set up for a flat (1-1) mapping, with TR=0.
;;; Function: load and start a DECB image (the FUZIX kernel). The
;;; location of the image on the SDcard is hard-wired by equates
;;; klba2..klba0 below.

klba2	equ $3                  ; LBA=0x0003.0400
klba1	equ $4
klba0	equ $0

;;; --------- multicomp i/o registers

;;; sdcard control registers
sddata	equ $ffd8
sdctl	equ $ffd9
sdlba0	equ $ffda
sdlba1	equ $ffdb
sdlba2	equ $ffdc

;;; vdu/virtual UART
uartdat	equ $ffd1
uartsta	equ $ffd0


;;; based on the memory map, this seems a safe place to load; the
;;; kernel doesn't use any space here. That may change and require
;;; a re-evaluation.
start   equ $d000


	org	start

;;; entry point
	lds	#stack

	lda	#'F'		; show user that we got here
	bsr	tovdu
	lda	#'U'
	bsr	tovdu
	lda	#'Z'
	bsr	tovdu
	lda	#'I'
	bsr	tovdu
	lda	#'X'
	bsr	tovdu

;;; decb format:
;;;
;;; section preamble:
;;; offset 0 0x00
;;;	   1 length high
;;;	   2 length low
;;;	   3 load address high
;;;	   4 load address low
;;;
;;; image postamble:
;;; offset 0 0xff
;;;	   1 0x00
;;;	   2 0x00
;;;	   3 exec high
;;;	   4 exec low

;;; Y - counts how many bytes remain to be transferred from disk;
;;; Start empty to trigger the initial disk load.
	ldy	#0

c@	jsr	getb		; get a byte in A from buffer
	cmpa	#$ff		; postamble marker?
	beq	post		; yes, handle it and we're done.
	;; expect preamble
	cmpa	#0		; preamble marker?
	lbne	abort		; unexpected.. bad format
	jsr	getw		; D = length
	tfr	d,x		; X = length
	jsr	getw		; D = load address
	tfr	d,u		; U = load address
	;; load section: X bytes into memory at U
d@	jsr	getb		; A = byte
	sta	,u+		; copy to memory
	leax	-1,x		; decrement byte count
	bne	d@		; loop for next byte if any
	bra	c@		; loop for next pre/post amble
	;; postable
post	jsr	getw		; get zero's
	cmpd	#0		; test D.. expect 0
	lbne	abort		; unexpected.. bad format
	jsr	getw		; get exec address
	pshs	d		; save on stack
        jsr     drain           ; leave SD controller idle
	rts			; go and never come back


;;; Abort! Bad record format.
abort	lda	#'B'		; show user that we got here
	bsr	tovdu
	lda	#'A'
	bsr	tovdu
	lda	#'D'
	bsr	tovdu
	lda	#$0d
	bsr	tovdu
	lda	#$0a
	bsr	tovdu
abort1	bra	abort1		; spin forever


;;;
;;; SUBROUTINE ENTRY POINT
;;; send character to vdu
;;; a: character to print
;;; can destroy b,cc

tovdu	pshs	b
vdubiz	ldb	uartsta
	bitb	#2
	beq	vdubiz	; busy

	sta	uartdat	; ready, send character
	puls	b,pc


;;;
;;; SUBROUTINE ENTRY POINT
;;; get next word from disk - trigger a new 512-byte read if necessary.
;;; return word in D
;;; must preserve Y which is a global tracking the bytes remaining

getw	jsr	getb		; A = high byte
	tfr	a,b		; B = high byte
	jsr	getb		; A = low byte
	exg	a,b		; flip D = next word
	rts


;;;
;;; SUBROUTINE ENTRY POINT
;;; get next byte from disk - trigger a new 512-byte read if necessary.
;;; return byte in A
;;; Destroys A, B.
;;; must preserve Y which is a global tracking the bytes remaining

getb	cmpy	#0      	; out of data?
	bne	getb4		; go read byte if not
getb2	bsr	read		; read next sector, reset Y
	ldd	lba1		; point to next linear block
	addd	#1
	std	lba1

getb4	lda	sdctl
	cmpa	#$e0
	bne	getb4		; byte not ready
	lda	sddata		; get byte
        leay    -1,y
done	rts


;;;
;;; SUBROUTINE ENTRY POINT
;;; read and discard any pending bytes - to leave the SD controller in
;;; a polite state.
;;; Destroys A, B, Y

drain	cmpy	#0      	; out of data?
	beq	done		; if so, finished

drain1	lda	sdctl
	cmpa	#$e0
	bne	drain1		; byte not ready
	lda	sddata		; get byte
        leay    -1,y
	bra     drain


;;;
;;; SUBROUTINE ENTRY POINT
;;; read single 512-byte block from lba0, lba1, lba2.
;;; Do not transfer any actual data,
;;; return Y showing how many bytes are available
;;; Destroys A, B
;;;

read    lda	sdctl
	cmpa	#$80
	bne	read            ; wait for previous command to complete

	lda	lba0		; load block address to SDcontroller
	sta	sdlba0
	lda	lba1
	sta	sdlba1
	lda	lba2
	sta	sdlba2

	clra
	sta	sdctl		; issue RD command to SDcontroller

	lda	#'.'		; indicate load progress
	lbsr	tovdu

	ldy	#512		; 512 bytes available
	rts

;;; location on SDcard of kernel (24-bit LBA value)
;;; hack!! The code here assumes NO WRAP from lba1 to lba2.
lba2	fcb     klba2
lba1	fcb     klba1
lba0	fcb     klba0

;;; 16 bytes of stack. Since we have plenty of space, reserve it within the
;;; 512 bytes of the boot loader itself. By inspection, the code uses 7 bytes
;;; of stack so this is more than generous (interrupts are disabled)
        fcb     0,0,0,0,0,0,0,0
        fcb     0,0,0,0,0,0,0,0
stack	equ	.


;;; Surprisingly, the org statement doesn't achieve this.. it
;;; doesn't pad the binary. Instead we need to calculate the
;;; padding that we want to introduce.
	zmb	start+$1b4-.


;;; For MBR and partition table formats, see:
;;; http://wiki.osdev.org/MBR_%28x86%29
;;; http://wiki.osdev.org/Partition_Table
;;; The mbr start sectors are RELATIVE TO THE START OF THIS MBR

        ;; offset $1b4
mbr_uid
	fcb     0,0,0,0,0       ; likewise, ".ds 10" does not occupy
        fcb     0,0,0,0,0       ; any space in the binary.

        ;; offset $1be
mbr_1
	fcb	$80		; bootable (but flag is ignored by FUZIX)
	fcb	$ff,$ff,$ff	; start: "max out" CHS so LBA values will be used.
	fcb	$01     	; system ID (not 0, not 0x5, 0xf or 0x85)
	fcb	$ff,$ff,$ff	; end: "max out" as before
	;; 32-bit values are stored little-endian: LS byte first.
	;; 65535-block root disk at 0x0003.1000 but this mbr
        ;; is at 0x0003.0000 so encode 0x0000.1000
	fcb	$00,$10,$00,$00	; partition's starting sector
	fcb	$fe,$ff,$00,$00	; partition's sector count

        ;; offset $1ce
mbr_2
	fcb	$80		; bootable (but flag is ignored by FUZIX)
	fcb	$ff,$ff,$ff	; start: "max out" CHS so LBA values will be used.
	fcb	$01     	; system ID (any non-zero value OK for FUZIX)
	fcb	$ff,$ff,$ff	; end: "max out" as before
	;; 32-bit values are stored little-endian: LS byte first.
	;; 65535-block additional disk at 0x0004.1000 but this mbr
        ;; is at 0x0003.0000 so encode 0x0001.1000
	fcb	$00,$10,$01,$00	; partition's starting sector
	fcb	$fe,$ff,$00,$00	; partition's sector count


        ;; offset $1de
mbr_3
	fcb	$80		; bootable (but flag is ignored by FUZIX)
	fcb	$ff,$ff,$ff	; start: "max out" CHS so LBA values will be used.
	fcb	$00     	; system ID (0=> unused)
	fcb	$ff,$ff,$ff	; end: "max out" as before
	;; 32-bit values are stored little-endian: LS byte first.
	fcb	$00,$01,$02,$fc	; partition's starting sector
	fcb	$00,$01,$02,$fc	; partition's sector count


        ;; offset $1ee
mbr_4
	fcb	$80		; bootable (but flag is ignored by FUZIX)
	fcb	$ff,$ff,$ff	; start: "max out" CHS so LBA values will be used.
	fcb	$00	        ; system ID (0=> unused)
	fcb	$ff,$ff,$ff	; end: "max out" as before
	;; 32-bit values are stored little-endian: LS byte first.
	fcb	$00,$10,$03,$00	; partition's starting sector
	fcb	$fe,$ff,$00,$00	; partition's sector count

        ;; offset $1fe
mbr_sig
	fcb	$55
	fcb	$aa

	end	start
