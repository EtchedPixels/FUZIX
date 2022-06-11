;
;	    Centurion platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_kernel_di
	    .export map_process
	    .export map_process_always
	    .export map_process_always_di
	    .export map_save_kernel
	    .export map_kernel_restore
	    .export map_process_x

	    ; DMA
	    .export _dma_set_length
	    .export _dma_set_base
	    .export _dma_set_mode0
	    .export _dma_enable

            ; exported debugging tools
            .export _plt_monitor
	    .export _plt_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

#include "kernel.def"
#include "../kernel-tms9995.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .common

_plt_monitor:
_plt_reboot:
	bra	 _plt_reboot

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;	TODO
;
;	Disable interrupts and return the old interrupt flag.
;

irqflag:
	.byte	0
	
___hard_di:
	di
	clrb	bl
	ldab	(irqflag)
	stbb	(irqflag)
	rsr
;
;	Enable interrupts
;
___hard_ei:
	ldab	1
	stab	(irqflag)
	ei
	rsr

___hard_irqrestore:
	ldab	2(s)
	stab	(irqflag)
	bz	eiret
	di
	rsr
eiret:
	ei
	rsr

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
	stmmu	0, kmap		; save the MMU properties
	rsr

init_hardware:
        ; set system RAM size
	lda	256
	sta	(_ramsize)
	lda	192
	sta	(_procmem)
        jmp	program_vectors_k

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .common

_program_vectors:
        ; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	;
	; our C caller will invoke us with the pointer C style in r4

	lda	2(s)
	rsr
	;
	; TODO
	;
program_vectors_k:
	rsr

;
;	Map routines - must save any register changed except A
;

;
;	Map the current process.
;
map_process_always:
map_process_always_di:
	lda	_udata + U_DATA__U_PAGE
	bra	map_bank_c
;
;	Map process in r0, use r0
;
map_process:
	ori	a,a
	bnz	map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	lda	kmap
	bra	map_bank
kmap:
	.ds	32		; 2K x 32 maps per ipl

map_process_x:
	; A holds the process
	ldb	P_TAB__P_PAGE_OFFSET
	add	b,a
map_process_2:
map_bank_c:
	; Copy the map into a scratch common space until we know more
	; about how the MMU load works (ie virtual v physical and is
	; the translation updated as one or as it goes
	;
	; Once we know what is actually required we can look at whether
	; to remove this or shuffle the MMU maps into common space or
	; use bcp here etc (we've got 512 bytes of maps so it's expensive
	; in common unless we go to bigger virtual page sizes)
	lda	(a)
	stx	(-s)
	stb	(-s)
	xfr	y,b
	stb	(-s)
	ldx	tmpmap
	xfr	a,y
	ldab	32
memcpl:
	ldb	(y+)
	stb	(x+)
	dcab
	bnz	memcpl
	ldb	(s+)
	xfr	b,y
	ldb	(s+)
	ldx	(s+)
	lda	tmpmap
map_bank:
	; We don't know if there is an A form of this from what we've
	; done so far, so self modify up the form we know.
	sta	(map_patch + 3);
map_patch:
	ldmmu	0,0	; 2E 0C F8 xx xx
	rsr

tmpmap:
	.ds	32
;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_kernel_restore:
	lda	saved_map
	jmp	map_bank

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	stmmu	0, saved_map
	lda	kmap
	bra	map_bank

saved_map:
	.ds	32

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	st	b,(-s)
outchar_l:
	ldbb	(0xF200)
	srr	b,2
	bnl	outchar_l
	stab	(0xF201)
	ld	b,(s+)
	rsr

;
;	DMA helpers
;
_dma_set_length:
	lda	2(s)
	lddmac	a
	rsr

_dma_set_base:
	lda	2(s)
	lddma	a
	rsr

_dma_set_mode0:
	ldab	2(s)
	dmamode 0
	rsr

_dma_enable:
	dmaen
	rsr
