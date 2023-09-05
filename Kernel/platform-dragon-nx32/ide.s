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
;
;	Not yet supporting swap
;
_devide_read_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endp
	tst _blk_op+2
	beq readword
	jsr map_proc_always
readword:
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	cmpx endp
	bne readword
	jsr map_kernel
	puls y,dp,pc

_devide_write_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endp
	tst _blk_op+2
	beq writeword
	jsr map_proc_always
writeword:
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	cmpx endp
	bne writeword
	jsr map_kernel
	puls y,dp,pc

endp:	.dw 0
