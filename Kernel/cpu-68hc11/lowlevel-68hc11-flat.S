
	.file "lowlevel-68hc11"
	.mode mshort


	.globl di
	.globl ei
	.globl irqrestore

	.globl common_syscall
	.globl dispatch_process_signal
	.globl common_intr
	.globl nmi_handler
	.globl trap_handler
	.globl doexec

	.globl outnewline
	.globl outcharhex
	.globl outstring
	.globl outx
	.globl outy
	.globl outd

	.globl preemption		; for debugging

#include "../build/kernel.def"
#include "kernel-hc11.def"


	.section .text

di:
	tpa		; return cc codes in D
	sei
	rts

ei:
	cli
	rts

irqrestore:		; D holds the return from di where A is the cc
	tap		; we trash overflow and carry but they are assumed
	rts		; clobbered anyway

;
;	Helper for byte swap
;
	.globl swab

swab:
	psha
	tba
	pulb
	rts


	.section .text

outnewline:
	ldab #0x0d
	bsr outchar_call
	ldab #0x0a
	bra outchar_call

outcharhex:
	pshb
	psha
	lsra
	lsra
	lsra
	lsra
	bsr outnibble
	pula
	psha
	bsr outnibble
	pula
	pulb
	rts

outnibble:
	anda #0x0F
	cmpa #0x0A
	blt outh2
	adda #0x07
outh2:	adda #0x30
	tab
outchar_call:
	jmp outchar

outstring:
	ldab ,x
	beq outsdone
	bsr outchar_call
	inx
	bra outstring

outx:
	pshx
	xgdx
	pshx		; actually the old D
	bsr outcharhex
	tba
	bsr outcharhex
	pulx
	xgdx
	pulx
outsdone:
	rts

outy:
	pshy
	xgdy
	pshy		; actually the old D
	bsr outcharhex
	tba
	bsr outcharhex
	puly
	xgdy
	puly
	rts

outd:
	psha
	pshb
	bsr outcharhex
	tba
	bsr outcharhex
	pulb
	pula
	rts

;
;	We are called from SWI. The stack holds 7 bytes of return
;	information, A holds the syscall number on entry, B the arg count
;	Arguments are left to right ordered.
;
;	On enry our frame if called from a 6800 series CPU (ACCB < 0x80). This
;	is the same as the 6800 itself but offset 2 bytes because the trap
;	frame also contains Y so is 2 bytes bigger on an HC11
;
;	14->	more arguments
;	13-12	last arg
;	11-10	return PC for caller to syscall
;	9-8	UserPC
;	7-6	Y
;	5-4	X
;	3	A
;	2	B
;	1	CC
;
;	We do TSX, ABX so that our arguments are the same X offset
;
;	On an HC11 the arguments are in 'classic' C order but there
;	is a hole caused by the fact that D is used to pass argument 1
;	and pushed by the stubs and a 2 byte return from the stubs to
;	the syscall wrappers
;
;	Thus:
;
;	19-18	Arg3
;	17-16	Arg2
;	15-14	Arg1
;	13-12	return PC for caller to syscall
;	11-10	Arg0
;	9-8	UserPC
;	etc for SWI
;
;	In thunked mode we are on the thunk stack in common space and
;	the data is passed by a bit of the common space. The common
;	user side coe does all the work
;
;
unix_syscall_entry:
	ldx	shared_sp	
	stx	udata+U_DATA__U_SYSCALL_SP	; Save user SP for brk checks
	pulx			; The only word on the shared stack is our
				; return and we need to save it as it will
				; be lost by the time we go back
	ldaa	shared_syscall
	staa	udata+U_DATA__U_CALLNO
	; All the hard work is done by the platform wrapper before
	; the call, so the argument marshalling for 680x v 68HC11
	; is done for us
	ldd	shared_argn
	std	udata + U_DATA__U_ARGN
	ldd	shared_argn1
	std	udata + U_DATA__U_ARGN+2
	ldd	shared_argn2
	std	udata + U_DATA__U_ARGN+4
	ldd	shared_argn3
	std	udata + U_DATA__U_ARGN+6
	ldaa	#1
	staa	udata+U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	lds	#kstack_top
	pshx				; save return ptr
	cli
	jsr	unix_syscall
	sei
	clr	udata+U_DATA__U_INSYS
	ldd	udata+U_DATA__U_RETVAL
	std	shared_retval
	ldd	udata+U_DATA__U_ERROR
	std	shared_error
	;	Pass back a user stack pointer to set
	ldx	udata + U_DATA__U_SYSCALL_SP
	stx	shared_sp
stub_return:
	ldab	udata+U_DATA__U_CURSIG
	stab	shared_signal
	beq	to_common
	clr	udata+U_DATA__U_CURSIG
	ldx	#udata+U_DATA__U_SIGVEC
	abx
	abx
	ldy	0,x
	beq	to_common
	clr	0,x
	clr	1,x
	sty	shared_sigvec
	; The stub code in the user bank gets passed the syscall
	; return and error plus the signal if any and vector. That
	; is sufficient for it to do all the dispatching
	; Get back the lost word for the shared stack and
	; jump back
to_common:
	pulx
	jmp	0,x


;
;	Call C code at Y from asm wrappers in an interrupt
;	Preserves D into the C code, and out of it
;
call_c_irq:
	; Save the C direct page values so we can re-enter along with the
	; four register variables we use.
	ldx *_.tmp
	pshx
	ldx *_.xy
	pshx
	ldx *_.z
	pshx

	ldx *_.d1
	pshx
	ldx *_.d2
	pshx
	ldx *_.d3
	pshx
	ldx *_.d4
	pshx

	ldx *_.frame
	pshx

	jsr ,y

	pulx
	stx *_.frame

	pulx
	stx *_.d4
	pulx
	stx *_.d3
	pulx
	stx *_.d2
	pulx
	stx *_.d1

	pulx
	stx *_.z
	pulx
	stx *_.xy
	pulx
	stx *_.tmp
	rts

;
;	The trap saved our entire register state for us but on the
;	user stack in the user bank. We have switched to the small
;	common stack at this point
;
interrupt_handler:
	; Save the return as the temp stack we were called on will
	; be lost in the pre-emption case
	pulx
	; Switch to the interrupt stack
	lds	#istack_top
	; Save return - this is also where the stub return
	; signal handling code wants it
	pshx

	ldaa	#1
	staa	udata+U_DATA__U_ININTERRUPT

	; Call the C code to process the interrupt
	ldy	#plt_interrupt
	jsr	call_c_irq

	; Was the interrupt to kernel code ?
	tst	udata+U_DATA__U_INSYS
	bne	iretk

	; Interrupts from user code may pre-empt
	tst	need_resched
	bne	preemption

	; Clean up
	clr	udata+U_DATA__U_ININTERRUPT
	clr	shared_preempt
	; Switch back to the stub stack
	; and do the signal bits
	bra	stub_return
iretk:
	pulx
	; No signal on this path
	clr	shared_signal
	clr	shared_preempt
	clr	udata+U_DATA__U_ININTERRUPT
	jmp	,x

;
;	We enter the pre-emption code with our RTI frame on the user stack
;	(not mapped) and running on the interrupt stack
;
;	TODO finish stack handling - need to sort out a way back down
;	the work stack which we lost in the pre-emption.
;
preemption:
	clr	need_resched
	; Save the stack pointer across as shared_sp is not per task
	ldd	shared_sp
	std	udata+U_DATA__U_SYSCALL_SP
	; Switch to our own process kernel stack while switched (the
	; shared and int stack will be used by other things in the meantime).
	; Get back the return address
	pulx
	lds	#kstack_top
	; 	Move it onto the kstack instead
	pshx

	ldab	#1
	stab	udata+U_DATA__U_INSYS

	; Signal check
	ldy	#chksigs
	jsr	call_c_irq

	ldx	udata+U_DATA__U_PTAB
	ldab	P_TAB__P_STATUS_OFFSET,x
	cmpb	#P_RUNNING
	bne	not_running
	; Move from running to ready state
	ldab	#P_READY
	stab	P_TAB__P_STATUS_OFFSET,x
	; If we are running set the punishment flag for CPU hogs
	bset	P_TAB__P_FLAGS_OFFSET,x #PFL_BATCH
not_running:
	; Task switch
	jsr	plt_switchout
	; Time will pass, things will run and eventually we will reappear
	; here when we get rescheduled
	; Interrupts will be back off
	clr	udata+U_DATA__U_ININTERRUPT
	clr	udata+U_DATA__U_INSYS
	ldx	udata+U_DATA__U_SYSCALL_SP
	;	So we come back on the right syscall or istack
	stx	shared_sp
	ldaa	#1
	staa	shared_preempt
	;	Kstack has X holding the return as desired
	bra 	stub_return

	.globl	set_cpu_type
	.globl	sys_cpu
	.globl	sys_cpu_feat
	.globl	sys_stubs

set_cpu_type:
	rts		; no variants to care about
sys_cpu:
	.byte	2	; 6800 class CPU
sys_cpu_feat:
	.byte	5	; 6800 with 6801/3 and 68HC11 features
sys_stubs:
	swi
	rts
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

/*
 *	This next section is the stub code run in the user map. It must
 *	access only user spare and shared common
 *
 *	Because the shared stacks vanish the callee saves the return
 *	address and jumps back. We need to then re-establish the stack
 */

	.section .common
/*
 *	An interrupt occurred. 
 */
common_intr:
	;	The caller will save this passed shared_sp and put it back
	;	on the return as needed
	sts	shared_sp
	lds	#int_stack	;	common interrupt stack (small be careful)
	jsr	map_save_kernel
	jsr	interrupt_handler
	lds	#int_stack
	ldaa	shared_preempt
	bne	was_preempt
	jsr	map_restore
	;	Back to the user stack
	lds	shared_sp
	bra	signal_out
was_preempt:
	jsr	map_proc_always
	lds	shared_sp
	bra	signal_out

common_syscall:
	;	Sort our the arguments and stash them in common
	;	space. interrupts must be off.
	sts	shared_sp	;	temporary stack save
	tsx
	staa	shared_syscall
	bitb	#0x80
	beq	syscall_0x
	ldd	11,x		;	13 is the return for the user
	std	shared_argn	;	layer stub
	ldd	15,x
	std	shared_argn1
	ldd	17,x
	std	shared_argn2
	ldd	19,x
	bra	syscall_main
syscall_0x:
	abx
	ldd	9,x
	std	shared_argn
	ldd	7,x
	std	shared_argn1
	ldd	5,x
	std	shared_argn2
	ldd	3,x
syscall_main:
	std	shared_argn3

	lds	#syscall_stack

	;	Now map and run the kernel code
	;	We are on the tiny shared stack

	;	We pass the user stack in shared_sp

	jsr	map_kernel

	jsr	unix_syscall_entry

	;	Put the stack back to do the remap
	lds	#syscall_stack
	jsr	map_proc_always

	;	Back to the user stack saved by
	;	the caller
	lds	shared_sp

	; Shared area holds stuff we need
	; We are on the user stack so have the rti frame to fiddle with
	tsx
	ldd	shared_retval
	stab	1,x
	staa	2,x
	ldd	shared_error
	std	3,x
signal_out:
	ldab	shared_signal
	beq	no_signal
	clra
	ldy	shared_sigvec
	pshy
	ldx	PROGLOAD+16
	; From this moment shared_* goes invalid
	cli
	jsr	,x
	ins	; drop the vector we pushed
	ins
no_signal:
	rti

doexec:
	sei
	clr	udata+U_DATA__U_INSYS
	ldx	udata+U_DATA__U_ISP
	lds	#syscall_stack		; Temp stack for the switch
	jsr	map_proc_always
	txs				; Set the stack up	
	xgdx				; execute address into x
	cli
	pshx
	pula				; high byte is our reloc base
	ins				; drop the other byte
	ldab	#DPBASE			; relocation info for DP
	ldy	#0			; clear register
	jmp	0,x			; from the caller D

;
trap_handler:
	; Were we in kernel - if so die horribly
	tst	udata + U_DATA__U_INSYS
	bne	trap_illegal
	tst	udata + U_DATA__U_ININTERRUPT
	bne	trap_illegal
	; Kick the user up the backside with a SIGILL. If the return from
	; it and didn't reset the handler then it will kill so it usually
	; does the right thing.
	ldx	#4			; SIGILL
	pshx
	ldx	udata+U_DATA__U_PTAB
	ldx	P_TAB__P_PID_OFFSET,x
	pshx				; expected hole
	pshx				; arg
	ldd	#0x3980			; kill, 68hc11 style call
	swi				; send ourselves a SIGILL
	rti				; and if we come back just
					; drop back to the expected frame

; These run with interrupts off on entry
trap_illegal:
	lds	#int_stack	
	jsr	map_kernel_di
	ldx	#illegalmsg
	jsr	outstring
	tsx
	ldx	8,x
	ldab	,x
	jsr	outcharhex
trapx:
	ldx	#atmsg
	jsr	outcharhex
	tsx
	ldx	8,x
	jsr	outx
	ldx	#nlmsg
	jsr	outstring
	jsr	plt_monitor

nmi_handler:
	lds	#int_stack	
	jsr	map_kernel_di
	ldx	#nmimsg
	jsr	outstring
	bra	trapx

illegalmsg:
	.ascii	"illegal instruction "
	.byte	0
atmsg:
	.ascii	" at "
nmimsg:
	.ascii	"[NMI]"
	.byte	0
nlmsg:
	.byte	13,10,0


/*
 * Support code chunks pulled from the gcc larith.asm and hacked about a bit
 */

/* libgcc routines for M68HC11 & M68HC12.
   Copyright (C) 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */


#define REG(NAME)			\
NAME:	.dc.w	1;			\
	.type NAME,@object ;		\
	.size NAME,2

/* Pseudo hard registers used by gcc.
   They should be located in page0.  */

	.sect .softregs
	.globl _.tmp
	.globl _.z,_.xy
REG(_.tmp)
REG(_.z)
REG(_.xy)
	.sect .softregs
	.globl _.frame
REG(_.frame)
	.sect .softregs
	.globl _.d1,_.d2
REG(_.d1)
REG(_.d2)
	.sect .softregs
	.globl _.d3,_.d4
REG(_.d3)
REG(_.d4)

	.sect .text

	.globl memcpy

memcpy:
	xgdy
	tsx
	ldd	4,x
	ldx	2,x	; SRC = X, DST = Y
	cpd	#0
	beq	End
	pshy
	inca			; Correction for the deca below
L0:
	psha			; Save high-counter part
L1:
	ldaa	0,x		; Copy up to 256 bytes
	staa	0,y
	inx
	iny
	decb
	bne	L1
	pula
	deca
	bne	L0
	puly			; Restore Y to return the DST
End:
	xgdy
	rts

	.globl memset

;;; D    = dst	Pmode
;;; 2,sp = src	SImode
;;; 4,sp = size
memset:
	xgdx
	tsy
	ldab	3,y
	ldy	4,y		; DST = X, CNT = Y
	beq	L3
	pshx
L2:
	stab	0,x		; Fill up to 256 bytes
	inx
	dey
	bne	L2
	pulx			; Restore X to return the DST
L3:
	xgdx
	rts

;
;	This one isn't taken from gcc	
;
	.globl strlen

strlen:
	xgdx
	ldd #0
L4:
	tst ,x
	beq L5
	inx
	addd #1
	bra L4
L5:	rts

;
;	Support routines (FIXME copy over)
;
	.globl ___ashrsi3
	.globl ___ashlsi3
	.globl ___lshlhi3
	.globl ___lshlsi3
	.globl ___lshrhi3
	.globl ___lshrsi3
	.globl ___one_cmplsi2
	.globl ___mulhi3


___ashrsi3:
	xgdy
	clra
	andb	#0x1f
	xgdy
	beq	retsrsi3
loopsrsi3:
	xgdx
	asra
	rorb
	xgdx
	rora
	rorb
	dey
	bne	loopsrsi3
retsrsi3:
	rts

___ashlsi3:
	xgdy
	clra
	andb	#0x1f
	xgdy
	beq	Return
loopslsi3:
	lsld
	xgdx
	rolb
	rola
	xgdx
	dey
	bne	loopslsi3
	rts

___lshlhi3:
	cpx	#16
	bge	Return_zero
	cpx	#0
	beq	retlhi3
looplhi3:
	lsld
	dex
	bne	looplhi3
retlhi3:
	rts
Return_zero:
	clra
	clrb
Return:
	rts

___lshrsi3:
	xgdy
	clra
	andb	#0x1f
	xgdy
	beq	Return
looprsi3:
	xgdx
	lsrd
	xgdx
	rora
	rorb
	dey
	bne	looprsi3
	rts

___lshrhi3:
	cpx	#16
	bge	Return_zero
	cpx	#0
	beq	Return
looprhi3:
	lsrd
	dex
	bne	looprhi3
	rts

___one_cmplsi2:
	comb
	coma
	xgdx
	comb
	coma
	xgdx
	rts

___mulhi3:
;
;
;  unsigned short ___mulhi3(unsigned short a, unsigned short b)
;
;	a = register D
;	b = register X
;
	;
	; 16 bit multiplication without temp memory location.
	; (smaller but slower)
	;
	pshx			; (4)
	ins			; (3)
	pshb			; (3)
	psha			; (3)
	pshx			; (4)
	pula			; (4)
	pulx			; (5)
	mul			; (10) B.high * A.low
	xgdx			; (3)
	mul			; (10) B.low * A.high
	abx			; (3)
	pula			; (4)
	pulb			; (4)
	mul			; (10) B.low * A.low
	pshx			; (4) 
	tsx			; (3)
	adda	1,x		; (4)
	pulx			; (5)
	rts			; (5) 20 bytes
				; ---
				; 91 cycles
