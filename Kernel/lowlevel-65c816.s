

	.P816
	.I8
	.A8
	
	.export _doexec
	.export interrupt_handler
	.export nmi_handler
	.export map_process_always
	.export map_kernel
	.export _userpage

	.export sigret_irq
	.export sigret
	.export syscall_vector

	.export illegal_inst
	.export trap_inst
	.export abort_inst
	.export emulation

	.export outstring
	.export outstringhex
	.export outnewline
	.export outcharhex
	.export outxa

	.export _need_resched

	.import istack_top
	.import istackc_top
	.import istack_switched_sp

	.import kstack_top
	.import kstackc_top

	.import _ssig

	.import outchar
	.import _kernel_flag
	.import _unix_syscall
	.import _platform_interrupt
	.import platform_doexec
	.import _inint
	.import _trap_monitor
	.import _switchout

	.import push0
	.import incaxy

	.include "platform/zeropage.inc"
	.include "platform/kernel.def"
	.include "kernel816.def"

	.code
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

syscall	=	$fe


;
;	Helper - given the udata bank in A set the DP value correctly. May
;	be worth optimizing one day. Must not corrupt X or Y
;
;	FIXME: there are various pieces of code that break if bank is > 0x80
;	and thus overflows. It might be cleaner if we re-arranged page here
;	and in the bank code as
;			code bank.8	}  For split I/D
;			data bank.8	}  0xFFFF = swapped?
;			dp.16		}  Our direct page (and swap)
;			(if swapped -> swap bank number)
;	that way doexec would do al the work along with bank65c816 and we
;	would be able to do things like sparse bank numbering in the C code
;
;	Even then we can't really make much use of 16MB of RAM because we
;	are worst case 128 processes with split I/D and we just don't have
;	enough bank 0 memory for that many DP and S pages!
;
;	(actually we could lazy swap DP and S up and down memory but really
;	who can use that much space except as a ramdisk ?)
;
;
setdp:
	php

	rep	#$30
	.a16
	.i16

	pha
	and	#$00FF			; clear top bits
	asl	a			; twice page (clears c)
	adc	#STACK_BANKOFF		; we now point at the stack
	inc	a			; plus 0x100
	xba				; Swap to get xx00 format we need
	tcd
	pla

	plp
	rts

	.a8
	.i8
;
;	This is called from a stub we put into each user process bank that
;	does a jsl to the kernel entry point then an rts. Must be called
;	in a8i8 state (or at least that's how you'll get the CPU back)
;
syscall_entry:
	sei				; interrupts off
	cld				; get into sane mode - no decimal, known state

	sep 	#$30
	.a8
	.i8

	txa
	sta	f:KERNEL_FAR+U_DATA__U_CALLNO
	cpy	#0
	beq	noargs

	; FIXME check if Y too large

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
	sta	f:KERNEL_FAR+U_DATA__U_ARGN,x
        inx
	inx
	cpy	#0
	bne	copy_args

noargs:
	sep	#$30
	.a8
	.i8

	lda	#KERNEL_BANK		; our direct and base bank need setting
	pha
	plb

	rep	#$30
	.a16
	.i16

	ldx	sp			; save the C stack before we swich DP

	lda	#KERNEL_DP
	tcd

	rep	#$30
	.a16
	.i16

	phx				; push user sp onto stack so we can
					; see it (only brk() needs it, nor
					; is it needed for save/restore)
	tsx
	stx	U_DATA__U_SYSCALL_SP
	ldx	#kstack_top-1
	txs				; set our CPU stack
	ldx	#kstackc_top-1		; set our C stack
	stx	sp			; 

	sep	#$30
	.a8
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

	sep	#$10
	.i8

	lda	U_DATA__U_CURSIG
	bne	signal_out

	pla				; pull the saved sp off the ustack
	pla				; discard it (it's still fine in
					; bank)
	rep	#$10
	.i16
	lda	U_DATA__U_PAGE
	tsx
	sta	3,x

	sep	#$10
	.i8

	ldy	U_DATA__U_RETVAL
	ldx	U_DATA__U_RETVAL+1
	; also sets z for us
	lda	U_DATA__U_ERROR
	pha				; save A

	lda	U_DATA__U_PAGE		; so we can set up DP
	pha
	plb				; User mapping for data bits
	jsr	setdp			; Set DP from A, corrupts A

	pla				; Sets Z based on error
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
	.a8
	.i8
	stz	U_DATA__U_CURSIG
	
	;
	;	Fixme stack correct U_DATA in x y a
	;

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
	; Stack the signal return (the signal itself can cause syscalls)
	; Be careful here stack ops are currently going to user stack but
	; data ops to kernel. Don't mix them up!
	pea	sigret
	phx				; signal number
	phy				; temporarily save C sp value
	asl a				; vector offset
	tay
	rep	#$30
	.a16
	.i16
	lda	#0
	ldx	U_DATA__U_SIGVEC,y	; get signal vector
	sta	U_DATA__U_SIGVEC,y	; and reset it
	ply				; get the temporary sp save back
	phx
	sep	#$20
	.a8
	; Needs to change for split I/D
	lda	U_DATA__U_PAGE		; target code page
	pha
	pea	PROGLOAD+20		; trap handler in user app
	pha
	plb				; get the right user data bank
	jsr	setdp
	sty	sp			; and finally restore the user C sp
	sep	#$10			; i8a8 on entry to handler
	.i8
	rtl			;	return into user app handler
;
;	doexec is a special case syscall exit path. Set up the bank
;	registers and return directly to the start of the user process
;
_doexec:
	sta	ptr1
	stx	ptr1+1		;	address to execute from
	sei
	stz	_kernel_flag

	rep	#$30
	.i16
	.a16

	ldx	ptr1		;	target address

	sep	#$20
	.a8

	lda	U_DATA__U_PAGE	;	get our bank
	asl	a
	adc	#STACK_BANKOFF	;	plus offset
	xba			;	swap to xx00
	lda	#$FF		;	stack top is xxff
	tcs			;	Set CPU stack at xxFF

	rep	#$20
	.a16

	inc			;	gives us the DP at xx+100
	pha
	pld

	;
	;	From this point onwards exec is referencing the user DP
	;	user S and kernel B (data)
	;

	sep 	#$20
	.a8

	; Set the C stack pointer
	ldy	U_DATA__U_ISP
	sty	sp		;	sp is in DP so we write user version
	; Fix up the syscall vector in ZP
	ldy	#syscall_vector
	sty	syscall		;	stores into user DP

	;
	;	From here we are entirely referencing user data but kernel
	;	code
	;
	;	(Will need to change for split I/D)

	lda	U_DATA__U_PAGE	;	bank for data
	pha			;	switch to userdata bank
	plb

	; lda U_DATA__U_PAGE+1	;	bank for code
	sty	a:syscall	;	and in bank:00FE

	pha			;	stack bank for rtl

	dex			;	Points to end of last instruction
				;	for a return
	phx			;	Execution address within bank
	lda	#0		;	
	tay			;	DP base is 0

	sep	#$30
	.a8
	.i8

	cli
	rtl

;
;	The basic idea of taking a trap and signalling is to make sure we
;	look like an interrupt and can re-cycle much of that code. The
;	caller already saved the CPU state. Interrupts are off at this point
;	as it we got here via a trap.
;
;	FIXME save the right registers to return correctly if caught
;
shoot_myself:
	sep	#$30
	.a8
	.i8
	lda	#KERNEL_BANK
	pha
	plb
	rep	#$30
	.a16
	.i16
	lda	#IRQ_DP
	tcd
	tsx
	stx 	istack_switched_sp
	ldx	#istack_top-1
	txs
	ldx	#istackc_top -1
	stx	sp

	;
	;	We have now arrived in kernel mappings for an interrupt
	;

	sep	#$30
	.a8
	.i8
	jsr	push0
	lda	#1
	sta	_inint
	pla			; top of stack is the saved signal number
	ldx	#0
	jsr 	_ssig
	jmp	join_interrupt_path

	
;
;	The C world here is fairly ugly. We have to stash various bits of
;	ZP on a 6502, fortunately on the 65C816 we can have a separate
;	interrupt DP
;
interrupt_handler:
	; Make sure we save all of the register bits
	rep	#$30
	.a16
	.i16
	pha
	phx
	phy
	phb
	phd
	cld			; no funnies with decimal

	; Now switch our data and DP to the kernel ones
	lda	#IRQ_DP		; interrupt has a private direct page
	tcd

	sep	#$20
	.a8
	lda	#KERNEL_BANK
	pha
	plb

	; Switch stacks
	tsx
	stx	istack_switched_sp
	ldx	#istack_top-1
	txs
	ldx	#istackc_top-1	; set up C istack (may not need eventually)
	stx	sp		

	sep	#$10
	.i8

	lda	#1
	sta	_inint
	jsr	_platform_interrupt

	;
	;	A synchronously delivered signal trap joins the interrupt
	;	return flow
	;

join_interrupt_path:
	stz	_inint

	; Restore the stack we arrived on

	rep	#$10
	.i16
	ldx	istack_switched_sp
	txs

	lda	_kernel_flag
	beq	ret_to_user

	;	Kernel interrupt path may change B and D itself so we must
	;	preserve them

	pld
	plb

	rep	#$20
	.a16

	ply
	plx
	pla
	rti

	.a8
	.i16

ret_to_user:
	lda	_need_resched
	beq	no_preempt

	lda	#0
	sta	_need_resched

	ldx	istack_switched_sp
	stx	U_DATA__U_SYSCALL_SP
	;
	;	Switch to our kernel stack (free because we are not
	;	pre-empting in a syscall
	;
	ldx	#kstack_top
	txs
	;
	;	Fix up the C stack
	;
	;
	;	Mark ourselves as in a syscall
	;
	lda	#1
	sta	U_DATA__U_INSYS
	;
	;	Drop back to a8i8 and schedule ourself out
	;
	sep	#$30
	.a8
	.i8
	lda	U_DATA__U_PTAB
	ldx	U_DATA__U_PTAB+1
	jsr	_switchout
	;
	;	We will (one day maybe) pop back out here. It's not
	;	guaranteed (we might be killed off)
	;
	lda	#0
	sta	U_DATA__U_INSYS
	;
	;	And exit the handler
	;
	rep	#$10

	.a8
	.i16
no_preempt:
	; Discard saved B and D - for user we will compute the correct
	; one (we could optimize this a shade and only throw on a
	; pre-empt FIXME)

	plx
	pla

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

	lda	U_DATA__U_PAGE

	tsx
	lda	U_DATA__U_PAGE	;+1 for split ID
	sta	9,x		; Might have moved bank if swapping

	sep	#$30
	.a8
	.i8
	pha
	plb	; Now data is the user bank

	jsr	setdp	; as is DP

	rep	#$30
	.a16
	.i16
	ply
	plx
	pla
	rti			; to bank we patched in

;
;	If we hit this we are on the user stack with all kernel mappings
;	and in a8/i16
;
	.a8
	.i16
signal_exit:
	stz	U_DATA__U_CURSIG
	sta	tmp1		; save signal 8bits (irq tmp1)


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
	pea	sigret_irq	; signal return
	sep	#$10
	.i8
	phy			; signal code
	tya
	asl a
	tay
	rep	#$30
	.a16
	.i16
	lda	#0
	ldx	U_DATA__U_SIGVEC,y
	sta	U_DATA__U_SIGVEC,y
	phx
	sep	#$20
	.a8
	lda	U_DATA__U_PAGE
	pha
	rep	#$30
	.a16 
	.i16
	ldx	#0
	txy
	pea	PROGLOAD+20
	sep	#$30
	.i8
	.a8
	lda	#$30		; i8a8
	pha
	rti

;
;	Trap handlers
;
illegal_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	ply
	cld
	lda	#4
sync_sig:
	sep	#$30
	.a8
	.i8
	pha
	lda	_kernel_flag
	bne	ktrap
	lda	_inint
	bne	itrap
	jmp	shoot_myself

itrap:
	lda	#<itrap_msg
	ldx	#>itrap_msg
outfail:
	jsr	outstring
	jmp	_trap_monitor
itrap_msg:
	.byte	"itrap!", 0

ktrap:
	lda	#<ktrap_msg
	ldx	#>ktrap_msg
	bra	outfail
ktrap_msg:
	.byte	"ktrap!", 0


trap_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	ply
	cld
	lda	#5
	bra	sync_sig

abort_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	ply
	cld
	lda	#7
	bra	sync_sig

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

	.a8
	.i8

map_process_always:
	lda U_DATA__U_PAGE
	sta _userpage
	rts
map_process:
	cmp #0
	bne map_process_2
	cpx #0
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

emulation:
	ldx #>emu_trap
	lda #<emu_trap
	jsr outstring
	jmp _trap_monitor
emu_trap:
	.byte "EM!", 0

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


;
;	Stubs get propogated into each segment at FF00
;
	.segment "STUBS"

	.a8
	.i8

sigret:
	sep #$30
	plx			; pull the return and error code back
	ply
	pla
	plp			; recover the flags
	rts			; back to the interrupted syscall return


sigret_irq:
	rep #$30
	.a16
	.i16
	ply
	plx
	pla
	plp			; flags and status bits (back to 8/8 usually)
	rts

	.a8
	.i8

syscall_vector:
	jsl	KERNEL_FAR+syscall_entry
	rts

;
;	Relocation stub (as it's easier to do relocation when in the user
;	bank than bouncing around and we have no common!). Called with i16
;	and X holding the binary start, tmp1 the shift and Y the code to
;	process
;
;	Returns with X pointing to end zero (so we can run it twice to do
;	ZP). We wipe the data as we go since it will become BSS.
;
;	Our relocation table lives in BSS start and is a byte table in the
;	format
;	0,0	end
;	0,n	skip n bytes and read next table entry
;	1-255	skip 1-255 bytes and relocate the byte now pointed at
;		then read next table entry
;
;	Our first table is high bytes of 16bit addresses to relocate (we
;	keep page alignment). Our second is ZP addresses to relocate.
;
;	Call with interrupts off until sure our irq code will get odd
;	code banks right.
;
;	FIXME: move out of stubs - run with code = normal dp = kernel
;	b = user and it'll work better
;
;	FIXME: consider checking if run off end with either X or Y
;
relocate:
	.i16
	.a8
	stz tmp2+1	; FIXME
reloc_loop:
	lda 0,x
	stz 0,x
	beq multi
	sta tmp2
	; skip 1-255 bytes and then relocate one byte
	rep #$20
	.a16
	tya
	adc tmp2
	tay
	sep #$20
	.a8
	; now relocate
	lda 0,y
	clc
	adc tmp1
	sta 0,y
	bra reloc_loop
multi:
	.i16
	.a8
	inx
	lda 0,x
	stz 0,x
	beq endofreloc		; 	(0,0 -> end)
	sta tmp2
	rep #$20
	.a16
	tya
	adc tmp2
	tay
	sep #$20
	.a8
	bra reloc_loop
endofreloc:
	rtl			; as called from bank KERNEL

