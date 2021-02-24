;
;	Glennside style IDE block transfer logic
;

	.module dragonide

	.globl _devide_read_data
	.globl _devide_write_data

	.globl _blk_op

        include "kernel.def"
        include "../kernel09.def"

	.area .common

;
;	Standad mapping for IDE
;
;
;	Not yet supporting swap
;
_devide_read_data:
	pshs y,dp
	lda #0xFE
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endp
	tst _blk_op+2
	beq readbyte
	jsr map_process_always
readbyte:
	lda <IDEDATA
	sta ,x++
	cmpx endp
	bne readbyte
	jsr map_kernel
	puls y,dp,pc

_devide_write_data:
	pshs y,dp
	lda #0xFE
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endp
	tst _blk_op+2
	beq writebyte
	jsr map_process_always
writebyte:
	lda ,x++
	sta <IDEDATA
	cmpx endp
	bne writebyte
	jsr map_kernel
	puls y,dp,pc

endp:	.dw 0
