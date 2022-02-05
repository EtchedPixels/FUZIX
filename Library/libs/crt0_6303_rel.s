;
;	6303 crt0 with relocations
;

	.setcpu 6303
	.code

	.export _environ
	.export head

	.bss
__bss_start:

	.code

head:
	.word	$80A8
	.byte	2		;	6800 series
	.byte	3		;	Needs 6803 and 6303 features
	.byte   0		;	Load page
	.byte	0		;	No hints
	.word	__code_size
	.word	__data_size
	.word	__bss_size
	.byte	<start		;	Offset to execute from
	.byte	0		;	No size hint
	.byte	0		;	No stack hint
	.byte	0		;	No hint bits

	.word   __sighandler	;	signal handler
	.word	0		;	relocations

;
;	This function is called when we need to deliver a signal. We can't
;	just blindly stack stuff as we can on big machines because we have
;	non-reentrancy issues in the compiler temporary and regvar usage
;
;	On entry
;	D = signal number
;	X = undefined
;
;	Return address is the correct route back to the kernel. Above it is
;	a copy of the vector, the signal number and an RTI frame.
;
__sighandler:
	; Save compiler temporaries and dp register variables
	ldx	@tmp
	pshx
	ldx	@tmp1
	pshx
	ldx	@tmp2
	pshx
	ldx	@tmp3
	pshx
	ldx	@tmp4
	pshx
	ldx	@fp
	pshx
	ldx	@reg
	pshx
	ldx	@reg+2
	pshx
	ldx	@reg+4
	pshx
	ldx	@sreg
	pshx

	; Arguments signal number and frame pointer
	pshb	; Save the signal number
	psha
	pshx
	tsx
	ldx	26,x	; vector
	jsr	,x
	; Discard arguments
	pulx
	pulx

	; Restore temporaries
	pulx
	stx	@sreg
	pulx
	stx	@reg+4
	pulx
	stx	@reg+2
	pulx
	stx	@reg
	pulx
	stx	@fp
	pulx
	stx	@tmp4
	pulx
	stx	@tmp3
	pulx
	stx	@tmp2
	pulx
	stx	@tmp1
	pulx
	stx	@tmp
	; Back to kernel provided address
	rts
;
;	On the 6803 this is a bit messy as we don't have two memory
;	pointers and we don't yet know what direct page is safe. Use
;	a helper to swap x and a temporary. We need this in the first
;	256 bytes

getb:
patch1:	stx	@0
patch2:	ldx	@2
	ldab	,x
	clr	,x
	inx
patch3:	stx	@2
patch4:	ldx	@0
	rts


;
;	Run a relocation set
;	On entry x is the relocation base and @0 the relocation byte
;	stream
;
reloc:	bsr	getb
	cmpb	#0x00
	beq	done
	cmpb	#0xff
	bne	relone
	; 0xFF means move on 0xFE bytes and don't do a relocation but
	; read a further relocation byte
	decb
	abx
	bra	reloc
relone:
	; 0x01-0xFE means move on that many bytes and then relocate that
	; byte
	abx
	tab
	addb	,x
	stab	,x
	bra	reloc
done:
	rts


start:
	psha	; save high byte of base address for second relocation set 
	pshb	; save DP relocation for the moment
	psha	; save high byte of base address again 
	pshb	; and DP again
	clrb	; D is now the base address
	pshb	; D into X without using DP
	psha
	pulx
	pulb	; Get the DP base
	;
	;	Patch the dp entries in the relocator (these are abs so
	;	will not be in the relocation table to mess up later)
	;	
	stab patch1-head+1,x
	stab patch4-head+1,x
	incb		; We use the first and second word
	incb		; of our available DP
	stab patch2-head+1,x
	stab patch3-head+1,x
	stab patch5-head+1,x

	ldaa	18,x	; relocation table base into A
	; We have to relocate this ourselves
	pulb	; get high byte of base address back
	aba
	ldab	19,x	; low byte does not need relocating
patch5:
	std	@2	; relocation table ptr into @2
	pula	; Recover the DP relocation value
	pshx	; save base
	bsr	reloc
	pulx	; restore base
	pula	; recover high byte of base copy (for second reloc run)
	bsr	reloc
	ldd	#__bss_start	; by now relocated
	addd	#__bss_size
	pshb
	psha
	jsr	_brk	; fix the brk base if it changed for relocations
	pulx

	clra
	clrb
	std	@zero
	incb
	stab	@one+1

	jsr	___stdio_init_vars
	tsx
	ldab	#4
	abx
	stx	_environ
	; Now call main
	decb		; In case someone defines it vararg! (4 bytes of arg)
	jsr	_main
	pshb
	psha
	jmp	_exit

	.bss
_environ:
	.word	0
