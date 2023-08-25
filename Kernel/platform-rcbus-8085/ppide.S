#include "../kernel-8085.def"

#define PPI_BUS_READ	0x92
#define PPI_BUS_WRITE	0x80

#define PPI_WR_LINE	0x20
#define PPI_RD_LINE	0x40

#define PPI_PORT_A		0x20
#define PPI_PORT_B		0x21
#define PPI_PORT_C		0x22
#define PPI_CTRL		0x23

#define IDE_REG_DATA		0x08
#define IDE_REG_STATUS		0x0F

	.setcpu 8085

	.export _ide_read

_ide_read:
	ldsi 2
	ldax d
	out PPI_PORT_C
	ori PPI_RD_LINE
	out PPI_PORT_C
	in PPI_PORT_A
	mov l,a
	ldax d
	out PPI_PORT_C
	ret

	.export _ide_write

_ide_write:
	mvi a,PPI_BUS_WRITE
	out PPI_CTRL
	ldsi 2
	ldax d			; register
	mov l,a			; save
	out PPI_PORT_C
	ldsi 4
	ldax d			; value
	out PPI_PORT_A
	xra a
	out PPI_PORT_B
	mov a,l
	ori PPI_WR_LINE
	out PPI_PORT_C
	mov a,l
	out PPI_PORT_C
	mvi a,PPI_BUS_READ
	out PPI_CTRL
	ret

	.common

	.export _devide_read_data

_devide_read_data:
	pop d
	pop h
	push h
	push d
	lda _td_raw
	ora a
	jz ide_rk
	dcr a
	jz ide_ru
	lda _td_page
	call map_for_swap
	jmp ide_rk
ide_ru:
	call map_proc_always
ide_rk:
	push b
	mvi b,0
	mvi a, IDE_REG_DATA
	out PPI_PORT_C			; Set up port ready for ready
	mov d,a
	ori PPI_RD_LINE
	mov e,a
readloop:
	mov a,e				; Read cycle
	out PPI_PORT_C
	in PPI_PORT_A			; Get data
	mov m,a
	inx h
	in PPI_PORT_B			; Get high byte
	mov m,a
	inx h
	mov a,d
	out PPI_PORT_C			; RD back high
	dcr b
	jnz readloop			; Done ?
	pop b
	jmp map_kernel


	.export _devide_write_data

_devide_write_data:
	pop d
	pop h
	push h
	push d
	lda _td_raw
	ora a
	jz ide_wk
	dcr a
	jz ide_wu
	lda _td_page
	call map_for_swap
	jmp ide_wk
ide_wu:
	call map_proc_always
ide_wk:
	push b
	mvi b,0
	mvi a, IDE_REG_DATA
	out PPI_PORT_C			; Set up port ready for ready
	mov d,a
	ori PPI_WR_LINE
	mov e,a
	mvi a, PPI_BUS_WRITE	; Set diredtion
	out PPI_CTRL
writeloop:
	mov a,d				; Write high
	out PPI_PORT_C
	mov a,m
	out PPI_PORT_A			; Put low byte
	mov m,a
	inx h
	mov a,m
	out PPI_PORT_B			; Put high byte
	inx h
	mov a,e				; Write low, strobe write
	out PPI_PORT_C
	dcr b
	jnz writeloop			; Done ?
	mov a,d
	out PPI_PORT_C			; WR high
	mvi a,PPI_BUS_READ
	out PPI_CTRL			; Bus direction to read
	pop b
	jmp map_kernel

	.discard

	.export _ppide_init

;
;	Initialize PPIDE. This consists of setting up the registers
;	for read state
;
_ppide_init:
	mvi a,PPI_BUS_READ
	out 0x23
	mvi a,IDE_REG_STATUS
	out 0x22
	ret
