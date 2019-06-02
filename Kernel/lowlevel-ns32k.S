#include "kernel-ns32k.def"
#
#	Low level support for NS32K
#
		.globl ___hard_di
		.globl ___hard_ei
		.globl ___hard_irqrestore
		.globl _doexec

		.globl outstring
		.globl outstringhex
		.globl outr0hex
		.globl outcharhex

		.globl _enable_icache
		.globl _flush_icache
		.globl _enable_dcache
		.globl _flush_dcache

		.globl _get_usp
		.globl _set_usp

		.globl _udata_shadow
#
#	Assume stack based C calls for now (we should revisit this ?)
#
_doexec:
		enter [],0
		movd 4(fp),r0
		# Put the supervisor stack back where we want it
		addr 1024(r6),r1
		lprd sp,r1
		lprd usp,U_DATA__U_ISP(r6)
		movqd 0,r2
		movqd 0,r3
		movqd 0,r4
		movqd 0,r5
		movqd 0,r6
		movqd 0,r7
		movd 0xB00,tos		# PSR and if we do DE mode module
		# We want U S I set (user mode, user stack, interrupt on
		movd r0,tos
		rett 0			# To user space

#
#	Later versions of the CPU will need these
#
_flush_icache:
#		cinv A,I,r0
#		ret 0
_flush_dcache:
#		cinv A,D,r0
#		ret 0
_enable_icache:
_enable_dcache:
		ret 0

_get_usp:
		sprd usp,r0
		ret 0
_set_usp:
		enter [],0
		movd 4(fp),r0
		lprd usp,r0
		ret 0
#
#	Interrupt control. The CPU can support fancy vectored interrupts
#	in which case you probably want to just mask the timer and stuff
#
___hard_di:
		sprw psr,r0
		movw r0,r1
		andw 0x800,r0
		andw 0xF7FF,r1
		lprw psr,r1
		ret 0
___hard_ei:
		sprw psr,r0
		movw r0,r1
		andw 0x800,r0
		orw 0x800,r1
		lprw psr,r1
		ret 0

___hard_irqrestore:
		enter [],0
		sprw psr,r0
		andw 0xF7FF,r0
		orw 4(fp),r0
		lprw psr,r0
		ret 0

outstring:
		enter [r0,r1,r2],0
		movb 0(r1),r0
		cmpqb 0,r0
		beq outdone
		jsr outchar
		addqd 1,r1
		br outstring
outdone:	exit [r0,r1,r2]
		ret 0

outstringhex:
		enter [r0,r1],0
outstringhexl:
		movb 0(r1),r0
		cmpqb 0,r0
		beq outdone
		bsr outcharhex
		addqd 1,r1
		br outstringhexl

outcharhex:
		movb r0,tos
		bsr outonehex
		movb tos,r0
outonehex:	andb 0xf0,r0
		lshb -4,r0
		addb '0',r0
		cmpb '9',r0
		ble digit
		addqb 7,r0
digit:		jump outchar
		
		
outr0hex:	
		enter [r0,r1,r2],0
		movd r0,r2
		lshd -12,r0
		bsr outcharhex
		movd r2,r0
		lshd -8,r0
		bsr outcharhex
		movw r2,r0
		lshw -4,r0
		bsr outcharhex
		movb r2,r0
		bsr outcharhex
		exit [r0,r1,r2]
		ret 0

		.data
		.align 2
_udata_shadow:	.long 0
_kernel_flag:	.byte 0
