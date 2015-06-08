; [fadd fdiv fdivr fmul fsub fsubr] [mem4r mem8r st,st(i) st(i),st]
	fadd	qword [ebx]
	fadd	dword [ebx]
	fadd	st,st(1)
	fadd	st(1),st
	fdiv	qword [ebx]
	fdiv	dword [ebx]
	fdiv	st,st(1)	; special swapping for this

; [faddp fdivp fdivrp fmulp fsubp fsubrp] st(i),st
	faddp	st(1),st

; [fbld fbstp] mem10r
	fbld	tbyte [ebx]
	fbstp	tbyte [ebx]

; [fcom fcomp] [mem4r mem8r optional-st(i)]
	fcom	dword [ebx]
	fcom	qword [ebx]
	fcom
	fcom	st(1)

; ffree st(i)
	ffree	st(1)

; [fucom fucomp fxch] optional-st(i)
	fucom
	fucom	st(1)

; [fiadd ficom ficomp fidiv fidivr fimul fist fisub fisubr] [mem2i mem4i]
	fiadd	word [ebx]
	fiadd	dword [ebx]

; [fild fistp] [mem2i mem4i mem8i]
	fild	word [ebx]
	fild	dword [ebx]
	fild	qword [ebx]

; [fld fstp] [mem4r mem8r mem10r st(i)]
	fld	dword [ebx]
	fld	qword [ebx]
	fld	tbyte [ebx]
	fld	st(1)

; [fldcw fnstcw] mem2i
	fldcw	word [ebx]
	fnstcw	word [ebx]

; [fldenv fnsave fnstenv frstor] mem
	fldenv	[ebx]
	fnsave	[ebx]
	fnstenv	[ebx]
	frstor	[ebx]

; fnstsw [mem2i ax]
	fnstsw	word [ebx]
	fnstsw	ax

; fst [mem4r mem8r st(i)]
	fst	dword [ebx]
	fst	qword [ebx]
	fst	st(1)

; fstcw mem2i (wait)
	fstcw	word [ebx]

; fstsw [mem2i ax] (wait)
	fstsw	word [ebx]
	fstsw	ax

; [fsave fstenv] mem (wait)
	fsave	[ebx]
	fstenv	[ebx]

; [fxxx] (no operands)
	fnop	; D9D0
	fchs	; D9E0
	fabs	; D9E1
	ftst	; D9E4
	fxam	; D9E5
	fld1	; D9E8
	fldl2t	; D9E9
	fldl2e	; D9EA
	fldpi	; D9EB
	fldlg2	; D9EC
	fldln2	; D9ED
	fldz	; D9EE
	f2xm1	; D9F0
	fyl2x	; D9F1
	fptan	; D9F2
	fpatan	; D9F3
	fxtract	; D9F4
	fprem1	; D9F5
	fdecstp	; D9F6
	fincstp	; D9F7
	fprem	; D9F8
	fyl2xp1	; D9F9
	fsqrt	; D9FA
	fsincos	; D9FB
	frndint	; D9FC
	fscale	; D9FD
	fsin	; D9FE
	fcos	; D9FF
	fucompp	; DAE9
	feni	; 9BDBE0
	fneni	; DBE0
	fdisi	; 9BDBE1
	fndisi	; DBE1
	fclex	; 9BDBE2
	fnclex	; DBE2
	finit	; 9BDBE3
	fninit	; DBE3
	fsetpm	; DBE4
	fcompp	; DED9
