;
;	For the Z180 we use the DMA engine for user copies except for the
;	byte/word requests. At the moment we don't re-route any small
;	requests to the uget/uput API if they are below the DMA break even
;	point or just small.
;
;	The DMA engine stalls the CPU so the fact we di/ei around this does
;	not actually make much real difference. If DMA latency is a problem
;	then it needs doing in blocks here and in z180.s for fork() which is
;	by far the longer case.
;
        .module usermem
	.z180

	.include "build/kernel.def"
        .include "cpu-z180/z180.def"
        .include "cpu-z80/kernel-z80.def"

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_proc_always
	.globl  map_kernel_restore

	.globl s__COMMONMEM

	.area _CODE

;
;	Compute the DMA pages to use. This isn't quite as trivial as it
;	looks because the kernel common flips per process
;
;
;	Copy BC bytes from HL to DE using the DMA engine
;
dma_to_kernel:
	ld a,#0x02		; memory inc to memory inc
	out0 (DMA_DMODE),a
	push bc
	ld a,(_udata + U_DATA__U_PAGE)	; Bank code >> 4 is the upper 4 bits
	rra
	rra
	rra
	rra
	ld c,a			; save src bank
	ld b,a			; possible dst bank if common
	ld a,d
	cp #(s__COMMONMEM >> 8)
	jr nc, dma_op
	ld b,#((OS_BANK + FIRST_RAM_BANK) >> 4)
	jr dma_op

dma_from_kernel:
	ld a,#0x02		; memory inc to memory inc
dma_user_fixed:
	out0 (DMA_DMODE),a	; burst mem to mem
	push bc
	ld a,(_udata + U_DATA__U_PAGE)	; get our bank
	rra
	rra
	rra
	rra
	ld c,a			; save src bank if common
	ld b,a			; save dst bank
	ld a,h
	cp #(s__COMMONMEM >> 8)
	jr nc, dma_op
	ld c,#((OS_BANK + FIRST_RAM_BANK) >> 4)	; not common
dma_op:
	; load banks
	out0 (DMA_SAR0B),c
	out0 (DMA_DAR0B),b
	; recover length
	pop bc
	; load DMA engine addresses and length
	out0 (DMA_BCR0H),b
	out0 (DMA_BCR0L),c
	out0 (DMA_DAR0H),d
	out0 (DMA_DAR0L),e
	out0 (DMA_SAR0H),h
	out0 (DMA_SAR0L),l
	ld a,#0x40		; burst transfer
	out0 (DMA_DSTAT),a
	; DMA stalls the ret until done
	ret


uputget:
        ; load DE with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
	; check if zero byte operation
	ld a, b
	or c
	di
	ret

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	; Z means nothing to copy
	call nz, dma_from_kernel
upop:
	ei
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget
	call nz, dma_to_kernel		; source in HL dest in DE, count in BC
	jr upop

;
;	Write first byte to 0 then DMA it over
;

	.ifne 0

__uzero:
	pop hl	; return
	pop de	; address
	pop bc	; size
	push bc
	push de
	push hl
	ld a, b	; check for 0 copy
	or c
	jr z,zerowipe
	ld hl,#zerowipe+1	; just need a random zero byte
	ld a,#0x0A		; memory const to memory inc
	di
	call dma_user_fixed
	ei
zerowipe:
	ld hl,#0
	ret

.else

	.area _COMMONMEM
;
;	We don't use the DMA for this at the moment. Some debug is needed
;	there. It's also not clear it is a speed win anyway.
;
__uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl
	push de
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_always
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out


.endif

;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

__uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
uputc_out:
	jp map_kernel_restore			; map the kernel back below common

__uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel_restore

__ugetc:
	call map_proc_always
        ld l, (hl)
	ld h, #0
	jp map_kernel_restore

__ugetw:
	call map_proc_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel_restore
