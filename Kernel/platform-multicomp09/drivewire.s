;;
;; DriveWire sector routines
;;
;; Copyright 2015 Tormod Volden
;; Copyright 2008 Boisy G. Pitre
;; Distributed under the GNU General Public License, version 2 or later.
;;

	; exported
	.globl _dw_operation
	.globl _dw_reset
	.globl _dw_transaction

	.area .common

;;;  Drivewire is really client (your retro 8 bitter)
;;;  driven.  Because the original CoCo hardware lacks
;;;  serial IRQs, and the fact that a 6809 interrupt on a ~2Mhz
;;;  machine isn't fast enough to catch the host's reply packet
;;;  , the write and read functions must be "close" together.
;;;   uint16_t dw_transaction( char *send, uint16_t scnt,
;;; 			       char *recv, uint16_t rcnt, uint8_t rawf )
;;;   x=send cc y ret scnt recv rcnt
;;;  Brett M. Gordon
_dw_transaction:
	pshs	cc,y		; save caller
	orcc	#0x50		; stop interrupts
	tstb			; rawflag?
	beq	skip@		; nope - then skip switching to process map
	jsr	map_process_always
skip@	ldy	5,s		; Y = number of bytes to send
	beq	ok@		; no byte to write - leave
	jsr	DWWrite		; send to DW
	ldx	7,s		; X is receive buffer
	ldy	9,s		; Y = number of bytes to receive
	beq	ok@		; no bytes to send - leave
	jsr	DWRead		; read in that many bytes
	bcs	frame@		; C set on framing error
	bne	part@		; Z zet on all bytes received
ok@	ldx	#0		; no error
out@	jsr	map_kernel
	puls	cc,y,pc		; return
frame@	ldx	#-1		; frame error
	bra	out@
part@	ldx	#-2		; not all bytes received!
	bra	out@


_dw_reset:
	; maybe reinitalise PIA here?
	; and send DW_INIT request to server?
	rts

_dw_operation:
	pshs y
	; get parameters from C, X points to cmd packet
	ldy 4,s		; driveptr
	lda ,y		; for now, contains minor = drive number directly
	ldb ,x		; write flag
	; buffer location into Y
	ldy 3,x
	; sector number into X
	ldx 1,x
	tstb
	bne @write
	jsr dw_read_sector
	bra @done
@write  jsr dw_write_sector
@done	bcs @err
	bne @err
	ldx #0
@ret	puls y,pc
@err	ldx #0xFFFF
	bra @ret

; Write a sector to the DriveWire server
; Drive number in A, sector number in X, buffer location in Y
; Sets carry or non-zero flags on error
dw_write_sector:
	; header: OP, drive = A, LSN 23-16 = 0, LSN 15-8 and LSN 7-0 = X
	clrb
	pshs a,b,x
	ldb #OP_WRITE
	pshs b
	; send header
	tfr s,x
	pshs y		; save buffer location
	ldy #5
	jsr DWWrite
	; send payload
	ldx ,s
	ldy #256
	jsr DWWrite
	; calculate checksum of payload, backwards
	exg x,y		; Y is zero after DWWrite
@sum	ldb ,-y
	abx
	cmpy ,s		; buffer location start
	bne @sum
	stx ,s		; checksum to send
	tfr s,x
	ldy #2
	jsr DWWrite
	; get status byte from server into following byte
	ldy #1
	clra		; clear carry bit for BECKER variant
	jsr DWRead
	leas 7,s
	bcs @ret
	bne @ret
	ldb -5,s	; received status byte (zero is success)
@ret	rts

;
; Based on "DoRead" by Boisy G. Pitre from DWDOS hosted on toolshed.sf.net 
; Read a sector from the DriveWire server
; Drive number in A, 16-bit sector in X, buffer location in Y
; Sets carry or non-zero flags on error

dw_read_sector:
         ; header: OP, drive = A, LSN 23-16 = 0, LSN 15-8 and LSN 7-0 = X
         clrb
         pshs  d,x,y
         lda   #OP_READEX
ReRead   pshs  a
         leax  ,s
	 ldy   #$0005
	 lbsr  DWWrite
	 puls  a
	 ldx   4,s			get read buffer pointer
	 ldy   #256			read 256 bytes
	 ldd   #133*1			1 second timeout
	 bsr   DWRead
         bcs   ReadEx
         bne   ReadEx
; Send 2 byte checksum
	 pshs  y
	 leax  ,s
	 ldy   #2
	 lbsr  DWWrite
	 ldy   #1
	 ldd   #133*1
	 bsr   DWRead
	 leas  2,s
	 bcs   ReadEx
	 bne   ReadEx
; Check received status byte
	 lda   ,s
	 beq   ReadEx
	 cmpa  #E_CRC
	 bne   ReadErr
	 lda   #OP_REREADEX
	 clr   ,s
	 bra   ReRead  
ReadErr  comb			; set carry bit
ReadEx	 puls  d,x,y,pc

; Used by DWRead and DWWrite
IntMasks equ   $50
NOINTMASK equ  1

; Hardcode these for now so that we can use below files unmodified
H6309    equ 0
BECKER   equ 1
ARDUINO  equ 0
JMCPBCK  equ 0
BAUD38400 equ 0

; These files are copied almost as-is from HDB-DOS
         include "dw.def"
         include "dwread.s"
         include "dwwrite.s"

