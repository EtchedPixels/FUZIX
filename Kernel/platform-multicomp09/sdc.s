;;;
;;;  multicomp09 SD Driver
;;;

;;; imported
	.globl blk_op		; blk operation arguments

;;; exported
	.globl _devsd_write
	.globl _devsd_read

        include "platform.def"

	section	.common

;;; Write 512 bytes to SDC
;;; the address and command have already been loaded: this
;;; only handles the data transfer.
;;;
;;; entry: x=data source
;;; can corrupt: a, b, cc, x
;;; must preserve: y, u
_devsd_write
	pshs	y
	ldy	#512		; 512 bytes
WrBiz	lda	SDCTL
	cmpa	#$a0
	bne	WrBiz		; space not available
	lda	,x+		; get byte from sector buffer
	sta	SDDATA		; store to SD
	leay	-1,y
	bne	WrBiz		; next
	puls	y,pc


;;; Read 512 bytes from SDC
;;; the address and command have already been loaded: this
;;; only handles the data transfer.
;;;
;;; entry: x=data destination
;;; can corrupt: a, b, cc, x
;;; must preserve: y, u
_devsd_read
	pshs	y
	ldy	#512		; 512 bytes
RdBiz	lda	SDCTL
	cmpa	#$e0
	bne	RdBiz		; byte not available
	lda	SDDATA		; get byte from SD
	sta	,x+		; store byte in sector buffer
	leay	-1,y
	bne	RdBiz		; next
	puls	y,pc
