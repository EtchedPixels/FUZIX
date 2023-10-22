#include "kernel-8085.def"

;
;	Simple implementation for now. Should be optimized
;

		.setcpu 8085

		.common

.export __uputc


.export __uputc

__uputc:
	ldsi 2
	ldax d
	ldsi 4
	lhlx
	mov d,a
	call map_proc_always
	mov m,d
	lxi h,0
	jmp map_kernel

.export __uputw

__uputw:
	ldsi 2
	lhlx		; data into HL
	push h
	ldsi 6		; 4 but we added a new 2 byte push
	lhlx		; address into HL
	xchg		; address now DE
	pop h		; data now HL
	call map_proc_always
	shlx		; (DE)=HL
	lxi h,0
	jmp map_kernel
	
.export __ugetc

__ugetc:
	ldsi 2
	lhlx
	call map_proc_always
	mov l,m
	mvi h,0
	jmp map_kernel

.export __ugetw

__ugetw:
	ldsi 2
	lhlx		; address into HL
	call map_proc_always
	xchg		; address now DE
	lhlx		; data into HL
	jmp map_kernel

.export __uget

;
;	Stacked arguments are src.w, dst.w, count.w
;
__uget:
	push b
	ldsi 8
	lhlx
	mov c,l
	mov b,h
	mov a,b
	ora c
	jz nowork
	ldsi 6
	lhlx
	push h
	ldsi 6		; offset 4 but we pushed another item so 6
	lhlx
	pop d		; HL = dest, DE = source, BC = count
	;
	;	So after all that work we have DE=src HL=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;
	dcx b
ugetcopy:
	call map_proc_always
	mov a,m
	call map_kernel
	stax d
	inx h
	inx d
	dcx b
	jnk ugetcopy
	pop b
	lxi h,0
	ret
nowork:
	pop b
	lxi h,0
	ret


.export __uput

__uput:
	push b
	ldsi 8
	lhlx
	mov c,l
	mov b,h
	mov a,b
	ora c
	jz nowork
	ldsi 6
	lhlx
	push h
	ldsi 6		; offset 4 but we pushed another item so 6
	lhlx
	pop d		; HL = dest, DE = source, BC = count
	;
	;	So after all that work we have DE=src HL=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;
	dcx b
uputcopy:
	mov a,m
	call map_proc_always
	stax d
	call map_kernel
	inx h
	inx d
	dcx b
	jnk uputcopy
	pop b
	lxi h,0
	ret


.export __uzero

__uzero:
	push b
	ldsi 6		; length
	xchg
	mov c,m		; check timing versus lhlx and moves
	inx h
	mov b,m
	ldsi 4
	lhlx		; HL is now pointer
	
	mov a,b
	ora c
	jz nowork

	call map_proc_always

	xchg		; pointer into DE so we can use SHLX

	lxi h,0

	mov a,b		; Divide length by 2
	rar
	mov b,a
	mov a,c
	rar
	mov c,a
	jnc nosingle	; If we have an odd byte zero it
	xra a
	stax d
	inx d
nosingle:		; Divide from words to dwords
	mov a,b
	rar
	mov b,a
	mov a,c
	rar
	mov c,a
	jnc nopair
	shlx		; Zero the odd word
	inx d
	inx d
nopair:
	mov a,b		; Divide from dwords to 8 bytes
	rar
	mov b,a
	mov a,c
	rar
	mov c,a
	jnc noquad
	shlx		; Zero the odd dword
	inx d
	inx d
	shlx
	inx d
	inx d
noquad:
	mov a,b		; Strip any rotated in carry bits
	ani 0x1F
	mov b,a
	ora c		; Check if there is any work left after we did the
			; small bits
	jz noquads
	dcx b		; NK works for FFFF not 0000
loop:
	shlx		; Finally let rip and zero each 8 byte block
	inx d
	inx d
	shlx
	inx d
	inx d
	shlx
	inx d
	inx d
	shlx
	inx d
	inx d
	dcx b
	jnk loop
noquads:
	call map_kernel	
	pop b
	ret
