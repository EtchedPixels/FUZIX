;
;	RCBUS CF Adapter IDE
;

	.module ide

	.globl _devide_read_data
	.globl _devide_write_data

	.globl _blk_op

        include "kernel.def"
        include "../../cpu-6809/kernel09.def"

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
	lda _blk_op+2
	beq readbyte
	deca
	beq readuser
	; Temporarily map the given page at 0x400
	lda _blk_op+3
	jsr map_for_swap
	bra readbyte
readuser:
	jsr map_proc_always
readbyte:
	lda <IDEDATA
	sta ,x+
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
	lda _blk_op+2
	beq writebyte
	deca
	beq writeuser
	; Temporarily map the given page at 0x400
	lda _blk_op+3
	jsr map_for_swap
	bra writebyte
writeuser:
	jsr map_proc_always
writebyte:
	lda ,x+
	sta <IDEDATA
	cmpx endp
	bne writebyte
	jsr map_kernel
	puls y,dp,pc

endp:	.dw 0
