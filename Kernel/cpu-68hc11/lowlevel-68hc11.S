
	.file "lowlevel-68hc11"
	.mode mshort


	.globl di
	.globl ei
	.globl irqrestore

	.globl unix_syscall_entry
	.globl dispatch_process_signal
	.globl interrupt_handler
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


	.section .common

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


	.section .common

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
unix_syscall_entry:
	tsx
	sts udata+U_DATA__U_SYSCALL_SP
	staa udata+U_DATA__U_CALLNO
	bitb #0x80			; 68HC11 or 680X call
	beq syscall_0x
	; Sane argument order but odd stack layout
	ldd 11,x
	std udata+U_DATA__U_ARGN
	ldd 15,x
	std udata+U_DATA__U_ARGN+2
	ldd 17,x
	std udata+U_DATA__U_ARGN+4
	ldd 19,x
	bra syscall_main
syscall_0x:
	abx
	ldd 9,x
	std udata+U_DATA__U_ARGN
	ldd 7,x
	std udata+U_DATA__U_ARGN+2
	ldd 5,x
	std udata+U_DATA__U_ARGN+4
	ldd 3,x
syscall_main:
	std udata+U_DATA__U_ARGN+6
	ldaa #1
	staa udata+U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	lds #kstack_top
	jsr map_kernel_di		; no-op in pure banked
	cli
	jsr unix_syscall
	sei
	clr udata+U_DATA__U_INSYS
	jsr map_proc_always	; no-op in pure banked
	lds udata+U_DATA__U_SYSCALL_SP
	; Now the fun bit - the CPU saves X and D so we have to patch them
	; into the return frame
	ldd udata+U_DATA__U_RETVAL
	tsx
	stab 1,x			; return in D
	staa 2,x
	ldd udata+U_DATA__U_ERROR
	std 3,x				; error in X
rti_return:
	ldab udata+U_DATA__U_CURSIG
	beq no_signal
	clr udata+U_DATA__U_CURSIG
	ldx #udata+U_DATA__U_SIGVEC
	abx
	abx
	ldy 0,x
	beq no_signal
	clr 0,x
	clr 1,x
	; Arguments to signal handler
	clra
	pshy
	ldx PROGLOAD+16	; Helper vector FIXME: assumes loaded at base
	; We are off the kernel stack at this point so we can
	; enable interrupts and leap into userspace
	cli
	jsr ,x
signal_return:
	ins	; Drop the signal number argument
	ins	; FIXME: shouldn't this be 2 bytes drop only CHECK
	ins	; And the vector
	ins
no_signal:
	rti

doexec:
	xgdx				; execute address into x
	sei
	clr udata+U_DATA__U_INSYS
	jsr map_proc_always
	lds udata+U_DATA__U_ISP
	des				; offset by 1 on the 680X
	cli
	pshx
	pula				; high byte is our reloc base
	ins				; drop the other byte
	ldab #DPBASE			; relocation info for DP
	ldy #0				; clear register
	jmp 0,x				; from the caller D

trap_handler:
	; Were we in kernel - if so die horribly
	tst udata + U_DATA__U_INSYS
	bne trap_illegal
	tst udata + U_DATA__U_ININTERRUPT
	bne trap_illegal
	; Kick the user up the backside with a SIGILL. If the return from
	; it and didn't reset the handler then it will kill so it usually
	; does the right thing.
	ldx #4				; SIGILL
	pshx
	ldx udata+U_DATA__U_PTAB
	ldx P_TAB__P_PID_OFFSET,x
	pshx				; expected hole
	pshx				; arg
	ldd #0x3980			; kill, 68hc11 style call
	swi				; send ourselves a SIGILL
	rti				; and if we come back just
					; drop back to the expected frame

trap_illegal:
	ldx #illegalmsg
	jsr outstring
	tsx
	ldx 8,x
	ldab ,x
	jsr outcharhex
trapx:
	ldx #atmsg
	jsr outcharhex
	tsx
	ldx 8,x
	jsr outx
	ldx #nlmsg
	jsr outstring
	jsr plt_monitor

nmi_handler:
	jsr map_kernel_di
	ldx #nmimsg
	jsr outstring
	bra trapx

illegalmsg:
	.ascii "illegal instruction "
	.byte 0
atmsg:
	.ascii " at "
nmimsg:
	.ascii "[NMI]"
	.byte 0
nlmsg:
	.byte 13,10,0


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

	jsr ,y

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
;	The trap saved our entire register state for us
;
;	At this point our user stack has the RTI frame on it, and the
;	interrupt stack is clear
interrupt_handler:
	sts istack_switched_sp

	; Switch to the interrupt stack
	lds #istack_top

	jsr map_save_kernel

	ldaa #1
	staa udata+U_DATA__U_ININTERRUPT

	; Call the C code to process the interrupt
	ldy #plt_interrupt
	jsr call_c_irq

	; Was the interrupt to kernel code ?
	tst udata+U_DATA__U_INSYS
	bne iretk

	; Interrupts from user code may pre-empt
	tst need_resched
	bne preemption

	; No pre-emption - map the process back
	jsr map_proc_always
	; Switch back to our kernel stack with the RTI frame
	lds istack_switched_sp
	; Clean up
	clr udata+U_DATA__U_ININTERRUPT
	tst udata+U_DATA__U_INSYS
	bne no_signals
	jmp rti_return
no_signals:
	rti
	;
	; Return to kernel mode
	; We don't check signals in this case and we use map_restore
	;
iretk:
	jsr map_restore
	lds istack_switched_sp
	clr udata+U_DATA__U_ININTERRUPT
	rti

;
;	We enter the pre-emption code with our RTI frame on the user stack
;	and running on the interrupt stack
;
preemption:
	clr need_resched
	; Save the stack pointer across as istack_switched_sp is not per
	; task
	ldd istack_switched_sp
	std udata+U_DATA__U_SYSCALL_SP
	; Switch to our own process kernel stack while switched (the
	; interrupt stack will be used by other things in the meantime).
	lds #kstack_top
	ldab #1
	stab udata+U_DATA__U_INSYS

	; Signal check
	ldy #chksigs
	jsr call_c_irq

	ldx udata+U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	; Move from running to ready state
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
	; If we are running set the punishment flag for CPU hogs
	bset P_TAB__P_FLAGS_OFFSET,x #PFL_BATCH
not_running:
	; Task switch
	jsr plt_switchout
	; Time will pass, things will run and eventually we will reappear
	; here when we get rescheduled
	clr udata+U_DATA__U_ININTERRUPT
	clr udata+U_DATA__U_INSYS
	; Map back in
	jsr map_proc_always
	; Return to our user stack
	lds udata+U_DATA__U_SYSCALL_SP
	; Check for signals
	ldaa udata+U_DATA__U_CURSIG
	beq no_signals
	jmp rti_return

	.section .text

	.globl set_cpu_type
	.globl sys_cpu
	.globl sys_cpu_feat
	.globl sys_stubs

set_cpu_type:
	rts		; no variants to care about
sys_cpu:
	.byte 2		; 6800 class CPU
sys_cpu_feat:
	.byte 5		; 6800 with 6801/3 and 68HC11 features
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
