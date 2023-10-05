;
;	Glenside style IDE block transfer logic
;

	.module dragonide

	.globl _devide_read_data
	.globl _devide_write_data

        include "kernel.def"
        include "../kernel09.def"

	.area .common
;
;	We don't really support swap properly but what we do is sufficient
;	for a simple memory mapping.
;
_devide_read_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	leay 512,x
	sty endp
readword:
	; word 1
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 2
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 3
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 4
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 5
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 6
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 7
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 8
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 8
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 9
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 10
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 11
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 12
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 13
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 14
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	; word 15
	lda <IDEDATA
	ldb <IDEDATA_L		; latched
	std ,x++
	cmpx endp
	bne readword
	puls y,dp,pc

_devide_write_data:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	leay 512,x
	sty endp
writeword:
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	ldd ,x++
	stb <IDEDATA_L
	sta <IDEDATA
	cmpx endp
	bne writeword
	jsr map_kernel
	puls y,dp,pc

endp:	.dw 0
