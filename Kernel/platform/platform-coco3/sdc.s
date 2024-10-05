;;;
;;;  The CoCoSDC Driver
;;;    Big thanks for Darren Atkinson for good documentation
;;;    and help with autodetection in the init method
;;;
;;; 

;;; exported
	.globl _devsdc_write
	.globl _devsdc_read

;;; dev

*********************************************************************
***    Hardware Addressing
*********************************************************************
CTRLATCH    equ    $FF40          ; controller latch (write)
CMDREG      equ    $FF48          ; command register (write)
STATREG     equ    $FF48          ; status register (read)
PREG1       equ    $FF49          ; param register 1
PREG2       equ    $FF4A          ; param register 2
PREG3       equ    $FF4B          ; param register 3
DATREGA     equ    PREG2          ; first data register
DATREGB     equ    PREG3          ; second data register



	section	.common



;;; Write 256 bytes from SDC
_devsdc_write
	pshs	y,u
	ldy	#PREG2		; set Y to point at data reg a
	ldd 	#64*256+4      	; A = chunk count (64), B = bytes per chunk (4)
	jsr	blkdev_rawflg	; map memory based on blkopt flag
wrChnk  ldu    	,x             	; get 2 data bytes from source
	stu    	,y             	; send data to controller
	ldu    	2,x            	; two more bytes..
	stu    	,y             	; ..for this chunk
	abx                   	; increment X by chunk size (4)
	deca                  	; decrement loop counter
	bne    	wrChnk         	; loop until all chunks written
	jsr	blkdev_unrawflg	; reset memory
	puls	y,u,pc		; return

;;; Reads 256 bytes from SDC
_devsdc_read
	pshs	y,u
	ldy	#PREG2	        ; set Y to point to data reg a
	ldd    	#32*256+8       ; A = chunk count (32), B = bytes per chunk (8)
	jsr	blkdev_rawflg	; map memory based on blkopt flag
rdChnk 	ldu    	,y              ; read 1st pair of bytes for the chunk
	stu    	,x              ; store to buffer
	ldu    	,y              ; bytes 3 and 4 of..
	stu    	2,x             ; ..the chunk
	ldu    	,y              ; bytes 5 and 6 of..
	stu    	4,x             ; ..the chunk
	ldu    	,y              ; bytes 7 and 8 of..
	stu    	6,x             ; ..the chunk
	abx                     ; increment X by chunk size (8)
	deca                    ; decrement loop counter
	bne    	rdChnk          ; loop if more chunks to read
	jsr	blkdev_unrawflg	; reset memory
	puls	y,u,pc		; return
