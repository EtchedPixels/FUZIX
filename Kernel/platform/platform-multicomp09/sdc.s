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
        tst     _blk_op+2       ; test user/kernel xfer
        beq     WrBiz           ; if zero then stay in kernel space
        jsr     map_proc_always ; else flip to user space
WrBiz	lda	SDCTL
	cmpa	#$a0
	bne	WrBiz		; space not available
	lda	,x+		; get byte from sector buffer
	sta	SDDATA		; store to SD
	leay	-1,y
	bne	WrBiz		; next
        puls    y
        jmp     map_kernel      ; reset to kernel space (tail optimise)


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
        tst     _blk_op+2       ; test user/kernel xfer
        beq     RdBiz           ; if zero then stay in kernel space
        jsr     map_proc_always ; else flip to user space
RdBiz	lda	SDCTL
	cmpa	#$e0
	bne	RdBiz		; byte not available
	lda	SDDATA		; get byte from SD
	sta	,x+		; store byte in sector buffer
	leay	-1,y
	bne	RdBiz		; next
        puls    y
        jmp     map_kernel      ; reset to kernel space (tail optimise)
