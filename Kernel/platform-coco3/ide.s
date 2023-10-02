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
;	Standad mapping for Glennside style IDE
;
_devide_read_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	jsr blkdev_rawflg
	leay 512,x
	sty endp
readword:
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	cmpx endp
	bne readword
	jsr blkdev_unrawflg
	puls y,dp,pc

_devide_write_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	jsr blkdev_rawflg
	leay 512,x
	sty endp
writeword:
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	cmpx endp
	bne writeword
	jsr blkdev_unrawflg
	puls y,dp,pc

endp:	.dw 0
