
	.file "lowlevel-68hc11"
	.mode mshort


	.globl di
	.globl ei
	.globl irqrestore

	.globl unix_syscall_entry
	.globl dispatch_process_signal
	.globl interrupt_handler

	.globl outnewline
	.globl outcharhex
	.globl outstring
	.globl outx
	.globl outy
	.globl outd

#include "platform/kernel.def"
#include "kernel-hc11.def"

	.section code

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

	.section common

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


outnewline:
	ldab #0x0d
	bsr outchar_call
	ldab #0x0a
	bra outchar_call


outcharhex:
	pshb
	lsrb
	lsrb
	lsrb
	lsrb
	bsr outnibble
	pulb
	pshb
	bsr outnibble
	pulb
	rts

outnibble:
	andb #0x0F
	cmpb #0x0A
	ble outh2
	addb #0x07
outh2:	addb #0x30
outchar_call:
	jmp outchar

outstring:
	ldab ,x
	beq outsdone
	bsr outchar_call
	inx
	bra outstring

outx:
	xgdx
	pshx		; actually the old D
	bsr outcharhex
	tab
	bsr outcharhex
	pulx
	xgdx
outsdone:
	rts

outy:
	xgdy
	pshy		; actually the old D
	bsr outcharhex
	tab
	bsr outcharhex
	puly
	xgdy
	rts

outd:
	psha
	pshb
	bsr outcharhex
	tab
	bsr outcharhex
	pulb
	pula
	rts


;
;	This is slightly odder than most platforms. At the point we
;	are called the arguments should already be in U_foo as we may be
;	doing a pure bank switch environment and will switch to the kernel
;	stack before this far call via the EEPROM
;
unix_syscall_entry:
	ldaa #1
	staa U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	jsr map_kernel		; no-op in pure banked
	cli
	jsr unix_syscall
	sei
	clr U_DATA__U_INSYS
	jmp map_process_always	; no-op in pure banked
	; signal processing happens in per platform code in case we are
	; pure banked
	; Caller must save errno and return value before invoking signal
	; processing.

;
;	May be a trampoline via a far call and bank switch, but not our
;	problem in this code. We do however assume the caller stack switched
;	for us to the IRQ stack
;
interrupt_handler:
	ldaa #1
	staa U_DATA__U_ININTERRUPT
	jsr map_save_kernel
	ldaa #1
	staa inint
	jsr platform_interrupt
	clr inint
	tst need_resched
	beq noswitch
	clr need_resched
	; Save the stack pointer across
	ldd istack_switched_sp
	std U_DATA__U_SYSCALL_SP
	lds #kstack_top
	jsr chksigs
	ldx U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
not_running:
	jsr platform_switchout
	jsr map_process_always
	; caller will switch back to stack in X
	ldx U_DATA__U_SYSCALL_SP
	rts
noswitch:
	jsr map_restore
	ldx istack_switched_sp
	rts			; caller will do the final stack flip

nmi_handler:
	lds #istack_top - 2
	jsr map_kernel
	ldx #nmimsg
	jsr outstring
	jsr platform_monitor

nmimsg:
	.ascii "[NMI]"
	.byte 13,10,0

;
;	Runs in kernel banking
;
dispatch_process_signal:
	ldab U_DATA__U_CURSIG
	bne dosig
	rts
dosig:
	clr U_DATA__U_CURSIG
	clra
	ldx #U_DATA__U_SIGVEC
	abx
	abx
	ldd ,x
	jmp sigdispatch		; platform provides. Calls d on the
				; user bank and stack. If it returns
				; then will go back via
				; dispatch_process_signal


;
;	Illegal instruction trap helper. Should send a signal
;	
sigill:
	rts
	
;
;	Helper for byte swap
;
	.globl swab

swab:
	psha
	tba
	pulb
	rts

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

	.globl memcpy

memcpy:
	xgdy
	tsx
	ldd	5,x
	ldx	3,x	; SRC = X, DST = Y
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
;;; 6,sp = size	HImode (size_t)
memset:
	xgdx
	tsy
	ldab	6,y
	ldy	7,y		; DST = X, CNT = Y
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
___ashlsi3:
___lshlhi3:
___lshlsi3:
___lshrsi3:
___lshrhi3:
___one_cmplsi2:
___mulhi3:
	rts

