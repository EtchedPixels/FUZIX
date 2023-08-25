#include "../kernel-8085.def"

		.code
		.setcpu 8085
.export init

	.word 0x8085

init:
        di
	; Make sure our interrupt is unmasked but the others are not
	mvi a,0x1D		; R7.5 | MSE | M7.5 | M 5.5
	sim
	lxi sp,kstack_top

        call init_early

	; Common is packed in the BSS space

	lxi b,__common_size
	lxi h,__bss
	lxi d,__common

	; Copy it high
nextbyte:
	mov a,m
	stax d
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz nextbyte

	; The discard follows the common

	lxi b, __discard_size
	lxi d, __discard
	dad b
	xchg
	dad b
	xchg
	jmp copydown
;
;	We copy discard from the top because it will probably overlap
;	on an 8080/8085 type system due to the larger code sizes.
;
nextbyted:
	mov a,m
	stax d
	dcx b
copydown:
	dcx h
	dcx d
	mov a,b
	ora c
	jnz nextbyted

	lxi b,__bss_size
	lxi h,__bss
wipe:
	mvi m,0
	inx h
	dcx b
	mov a,b
	ora c
	jnz wipe

;
;	Now sort out the common across banks. We need to do this with our
;	low memory pinned to the first bank
;
	lxi d,__common
nextword:
	mvi a,0x03		; read common RAM0 keep low RAM0
	out 0xFF
	lhlx
	mvi a,0x43		; write common RAM1 low RAM0
	out 0xFF
	shlx
	mvi a,0x13		; RAM2
	out 0xFF
	shlx
	mvi a,0x53		; RAM3
	out 0xFF
	shlx
	mvi a,0x07		; RAM4
	out 0xFF
	shlx
	mvi a,0x47		; RAM5
	out 0xFF
	shlx
	mvi a,0x17		; RAM6
	out 0xFF
	shlx
	mvi a,0x57		; RAM7
	out 0xFF
	shlx
	inx d
	inx d
	xra a
	cmp d
	jnz nextword

	; Set up bankid for the paging code

	lxi h,bankid
	mvi a,0x03
	out 0xFF
	mov m,a
	mvi a,0x43
	out 0xFF
	mov m,a
	mvi a,0x13
	out 0xFF
	mov m,a
	mvi a,0x53
	out 0xFF
	mov m,a
	mvi a,0x07
	out 0xFF
	mov m,a
	mvi a,0x47
	out 0xFF
	mov m,a
	mvi a,0x17
	out 0xFF
	mov m,a
	mvi a,0x57
	out 0xFF
	mov m,a

	; Set up memory mapping for kernel entry
	mvi a,0x57		; kernel, common of  init
	sta curmap
	out 0xFF		; Back to RAM 0, 0

        call init_hardware

        call _fuzix_main
        di
stop:   hlt
        jmp stop

