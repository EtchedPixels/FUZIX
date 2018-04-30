;;; Low level common memory transfer routine
;;; for Roger Taylor's CoCo On A Chip SD


;;; imported
	.globl blk_op

;;; exported
	.globl _devrtsd_write
	.globl _devrtsd_read

DATA	equ	$ff70
STATUS  equ	$ff71


	section	.common

;;; fixme: unroll loops
_devrtsd_write
	pshs	u
	ldu	#DATA		; set Y to point at data reg a
	clra
	jsr	blkdev_rawflg	; map memory based on blkopt flag
a@	ldb	STATUS
	lsrb
	bcs	a@
	ldb    	,x+            	; get 2 data bytes from source
	stb    	,u             	; send data to controller
	deca                  	; decrement loop counter
	bne    	a@         	; loop until all chunks written
	jsr	blkdev_unrawflg	; reset memory
	puls	u,pc		; return
	
_devrtsd_read
	pshs	u
	ldu	#DATA	        ; set Y to point to data reg a
	clra
	jsr	blkdev_rawflg	; map memory based on blkopt flag
a@	ldb	STATUS
	bpl	a@
	ldb	,u
	stb	,x+
	deca                    ; decrement loop counter
	bne    	a@              ; loop if more chunks to read
	jsr	blkdev_unrawflg	; reset memory
	puls	u,pc		; return