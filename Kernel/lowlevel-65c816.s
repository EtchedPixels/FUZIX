

	.P816
	.I8
	.A8
	
	.export unix_syscall_entry
	.export _doexec
	.export interrupt_handler
	.export nmi_handler

	.export outstring
	.export outstringhex
	.export outnewline
	.export outcharhex
	.export outxa

	.export _userpage

	.export _need_resched

	.import outchar
	.import _kernel_flag
	.import _unix_syscall
	.import map_restore
	.import map_save
	.import map_process_always
	.import map_kernel
	.import _platform_interrupt
	.import platform_doexec
	.import _inint
	.import _trap_monitor

	.include "platform/zeropage.inc"
	.include "platform/kernel.def"
	.include "kernel816.def"

	.segment "COMMONMEM"
;
;	Unlike Z80 we need to deal with systems that have no overlapping
;	memory banks. We pass the arguments is a single pointer therefore
;	we have to copy them interbank
;
;	Called with interrupts off, on the kernel stack
;	On completion U_DATA__U_ERROR an U_DATA__U_RETVAL hold the returns
;
;	Caller is expected to set 65C816 to I8A8
;
;	FIXME: do we want 6502 binaries syscall or to implement a cleaner
;	65c816 brk based syscall ?
;
;
;	Assumptions:
;	No split I/D	: code = data = direct page
;			  This could be fixed by making page track two
;			  values and keep them in ->page.
;
;
;	TODO:
;	- add vectors and stubs to bank 0 setup, and to other banks for
;	  stubs (syscall and signal return)
;	- audit and begin testing
;
;
;	This is called from a stub we put into each user process bank that
;	does a jsl to the kernel entry point then an rts
;
syscall_entry:
	php				; save cpu mode of caller on ustack
	sei				; interrupts off
	cld				; get into sane mode - no decimal, known state
	sep 	$30
	.a8
	.i8
	stx	U_DATA__U_CALLNO
	cpy	#0
	beq	noargs

	lda	sp
	sta	ptr1
	ldx	sp+1
	stx	ptr1+1
	jsr	incaxy
	sta	sp
	stx	sp+1

	;
	;	We copy the arguments but need to deal with the compiler
	;	stacking in the reverse order. At this point ptr1 points
	;	to the last byte of the arguments (first argument). We go
	;	down the stack copying words up the argument list.
	;
	;	Would it be saner to invert the struct fields on LTR
	;	stacking platforms and use mvn ???? FIXME
	;
	ldx #0
copy_args:
	rep	#$20
	.a16
	dey
	dey
	lda	(ptr1),y		; copy the arguments over
	ld
	sta	KERNEL_BANK:U_DATA__U_ARGN,x
        inx
	inx
	cpy	#0
	bne	copy_args

noargs:
	rep	#$30
	.a16
	.i16
	lda	#KERNEL_BANK		; our direct and base bank need setting
	pha
	plb
	lda	#KERNEL_DP
	tcd
	rep	#$30
	.a8
	.i8
	rep	#$10
	.i16
	ldx	sp			; save the C stack
	pshx				; on the main stack
	; FIXME: how to check stack space in brk when this is not visible ?
	tsx
	stx	U_DATA__U_SYSCALL_SP
	ldx	#kstack_top
	txs				; set our CPU stack
	ldx	#kstack_c		; set our C stack
	stx	sp			; 
	cli				; sane state achieved

	sep	#$10
	.i8
	lda	#1
	sta	_kernel_flag		; In kernel mode
	cli				; Interrupts now ok
	jsr	_unix_syscall		; Enter C space
	sei				; Interrupts back off
	stz	_kernel_flag
	rep	#$10
	.i16
	ldx	U_DATA__U_SYSCALL_SP
	txs				; Back to the old stack
					; We can't restore sp yet as we are
					; in the wrong bank
	.i8
	sep	#$10
	lda	U_DATA__U_CURSIG
	bne	signal_out
	sep	#$20
	lda	U_DATA__U_PAGE
	.a16
	;
	;	We use this trick several times. U_PAGE holds the bank
	;	register. As we want our DP to be bank:0000 we need to
	;	shift left 8 and add 00 to get our DP value
	;
	pha
	plb				; User mapping for data bits
	xba				; from xx to xx00 for DP
	lda	#0
	tcd

	pla				; pull the saved sp off the ustack
	sta	sp			; and store it 16bit

	rep	#$20
	.a8 
	plp				; off ustack
	; We may now be in decimal !
	ldy	U_DATA__U_RETVAL
	ldx	U_DATA__U_RETVAL+1
	; also sets z for us
	lda	U_DATA__U_ERROR
	rtl

	;
	;	The goal of the signal handler is to push a frame onto
	;	the stack that causes us to return from the kernel to user
	;	space PROGLOAD+0x20 (0x220) with a stack frame holding
	;	the desired vector and a return path to a cleanup routine
	;	to recover stuff
	;
	;	The tricky bit here is keeping the user C sp somewhere safe
	;	we pop it into y and carefully juggle it around until we can
	;	write into the correct DP
	;
	;	The sigret code is stuffed into each user bank, and it
	;	does the following
	;
	;	sep #$30
	;	plx	recover return and error codes
	;	ply
	;	pla
	;	plp	recover cpu state
	;	rts	in bank return to the interruption point
	;
signal_out:
	clz	U_DATA__U_CURSIG
	
	rep	#$10
	.i16
	ply				; user stack pointer to go to sp
	tsx
	dex				; move past existing frame P
	dex				; return PC
	dex				; return PC
					; lose the bank
	txs
	sep	#$10
	.i8
	tax				; Save signal code in X
	; Stack the signal return (the signal itself can cause syscalls)
	; Be careful here stack ops are currently going to user stack but
	; data ops to kernel. Don't mix them up!
	lda	#>sigret		; needs to be a fixed address in user
	pha
	lda	#<sigret		; FIXME
	pha
	phx				; signal number
	phy				; temporarily save C sp value
	txa
	asl a				; vector offset
	tay
	rep	#$10
	.i16
	ldx	U_DATA__U_SIGVEC,y	; get signal vector
	clz	U_DATA__U_SIGVEC,y	; and reset it
	ply				; get the temporary sp save back
	phx
	; Needs to change for split I/D
	lda	U_DATA__U_PAGE		; target code page
	pha
	pea	#PROGLOAD+20		; trap handler in user app
	pha
	plb				; get the right user data bank
	xba				; get into ff00 format
	lda	#0
	tcd				; set user DP correctly
	sty	sp			; and finally restore the user C sp
	sep	#$10			; i8a8 on entry to handler
	.i8
	rtl			;	return into user app handler
;
;	doexec is a special case syscall exit path. Set up the bank
;	registers and return directly to the start of the user process
;
_doexec:
	; FIXME how do we find the correct cpu stack for this process
	;
	; Would (bank + offset) << 8 work ?
	sta	_tmp1
	stx	_tmp1+1
	sei
	stz	_kernel_flag
	rep	#$30
	.i16
	.a16
	ldx	_tmp1		;	target address
	sep	#$20
	.a8
	lda	U_DATA__U_PAGE	;	get our bank
	clc
	adc	#STACK_BANKOFF
	xba			;	swap to xx00
	lda	#FF		;	stack top is xxff
	tcs			;	Set CPU stack at xxFF
	; Split I/D will need to change this logic
	lda	U_DATA__U_PAGE	;	our bank
	xba			;	now in the form xx00 as we need
	tcd			;	set the user DP
	; We are now on the correct DP and CPU stack
	ldy	U_DATA__U_ISP
	sty	sp		;	sp is in DP so we write user version
	sep	#$30
	.i8
	.a8
	; Will need to change for split I/D
	lda	U_DATA__U_PAGE	;	bank
	pha			;	switch to userdata bank
	plb
	pha			;	stack bank for rtl
	rep	#$10
	.i16
	phx			;	Execution address within bank
	cli
	lda	#0		;	
	tay			;	DP base is 0
	sep	#$10
	.i8
	rtl
	
;
;	The C world here is fairly ugly. We have to stash various bits of
;	zero page magic because its not re-entrant.
;
interrupt_handler:
	; Make sure we save all of the register bits
	rep	#$30
	.a16
	.i16
	pha
	phx
	phy
	cld			; no funnies with decimal

	; Now switch our data and DP to the kernel ones
	lda	#KERNEL_BANK
	pha
	plb
	lda	#IRQ_DP		; interrupt has a private direct page
	tcd

	; Switch stacks
	tsx
	stx	istack_switched_sp
	ldx	#istack
	txs
	ldx	#istack_c	; set up C istack (may not need eventually)
	stx	sp		

	sep	#$30

	.a8
	.i8

	lda	#1
	sta	_inint
	jsr	_platform_interrupt
	stz	_inint

	; Restore the stack we arrived on

	rep	#$10
	.i16
	ldx	istack_switched_sp
	txs
	; TODO .. pre-emption


	; Signal return path
	; The basic idea here is that if a signal is pending we
	; build a new stack frame under the real one and rti to that. The
	; hook code in low user memory will then clean up the real frame
	lda	U_DATA__U_CURSIG
	bne	signal_exit

	;
	;	Switch banks correctly and return to user. We can't just
	;	save the bank as a task switch or swap might move the
	;	process
	;
	lda	U_DATA_U_PAGE
	sep	#$30

	.a8
	.i8

	pha
	plb	; Now data is the user bank
	xba
	lda	#0
	tcd	; and DP is right

	rep	#$30
	.a16
	.i16

	ldx	istack_switched_sp
	txs
	ply
	plx
	pla
	rti

;
;	If we hit this we are on the user stack with all kernel mappings
;	and in a8/i16
;
	.a8
	.i16
signal_exit:
	clz	U_DATA__U_CURSIG
	sta	_tmp1		; save signal 8bits (irq tmp1)


	; Move down the stack frame
	;
	; Right now it looks like this
	;
	;		Bank
	;		PC high
	;		PC low
	;		P
	;		A16
	;		X16
	;		Y16
	;	SP
	;
	; We want it to look like
	;		PC high
	;		PC low
	;		P
	;		A16
	;		X16
	;		Y16
	;		error
	;		retval
	;		sigret_irq
	;		signal number
	;
	rep #$30
	.i16
	.a16

	tsc			; stack to accumulator
	clc
	adc	#10		; top of block to change
	tax
	tay
	dex			; source one below dest
	lda	#8		; copy 9 bytes
	mvp	0,0
	;
	;	At this point y points to the byte below the last
	;	destination copied (aka s)
	;
	tya
	tcs	; stick y in s

	sep	#$20
	.a8
	;
	;	Stack the signal bits
	;
	lda	U_DATA__U_ERROR
	pha
	lda	U_DATA__U_RETVAL
	pha
	lda	U_DATA__U_RETVAL+1
	pha
	pea	#sigret_irq
				; sig	nal return
	sep	#$10
	.i8
	phy			; signal code
	tya
	asl a
	tay
	rep	#$30
	.i16
	.a16
	lda	U_DATA__U_SIGVEC,y
	clz	U_DATA__U_SIGVEC,y
	pha
	sep	#$20
	.a8
	lda	U_DATA__U_PAGE
	pha
	rep	#$30
	.a16 
	.i16
	ldx	#0
	txy
	pea	#PROGLOAD+20
	sep	#$30
	.i8
	.a8
	lda	#$30		; i8a8
	pha
	rti
;
;	We can make the map routines generic as all 65c816 are the same
;	mapping model. We don't change anything but instead track the page
;	we need to use for the'far' operations.
;
;	We don't actually make any use of this code in the base 65c816
;	image but it is there and needed for drivers that do swapping.
;
;	As we can't map the bank into kernel memory we'll actually need
;	some custom swap logic in the disk drivers which is sucky but I
;	don't currently have any better idea!
;
map_process_always:
	lda U_DATA__U_PAGE
	sta _userpage
	rts
map_process:
	cmp #0
	bne map_process_2
	cmpx #0
	bne map_process_2
map_kernel:
	rts
map_process_2:
	sta ptr1
	stx ptr1+1
	lda (ptr1)	; 4 bytes if needed
	sta _userpage
	rts
_userpage:
	.byte 0

nmi_handler:
	sep #$30
	ldx #>nmi_trap
	lda #<nmi_trap
	jsr outstring
nmi_stop:
	jmp _trap_monitor
nmi_trap:
	.byte "NMI!", 0

outstring:
	sta ptr1
	stx ptr1+1
	ldy #0
outstringl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	jsr outchar
	iny
	jmp outstringl

outstringhex:	; string in X,A
	sta ptr1
	stx ptr1+1
	ldy #0
outstringhexl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	jsr outcharhex
	iny
	jmp outstringhexl

outnewline:
	pha
	lda #10
	jsr outchar
	lda #10
	jsr outchar
	pla
outdone1:
	rts

outcharhex:
	pha
	and #$f0
	lsr a
	lsr a
	lsr a
	lsr a
	cmp #10
	bcc deci1
	clc
	adc #7
deci1:
	clc
	adc #48			; ascii zero
	jsr outchar
	pla
	and #$0f
	cmp #10
	bcc deci2
	clc
	adc #7
deci2:
	clc
	adc #48
	jmp outchar

outxa:	pha
	txa
	jsr outcharhex
	pla
	jmp outcharhex

_need_resched:
	.byte 0
