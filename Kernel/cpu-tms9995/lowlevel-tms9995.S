#include "../build/kernel.def"
#include "kernel-tms9995.def"

	.code

	.export _sys_cpu
	.export _sys_cpu_feat
	.export	_set_cpu_type
	.export _sys_stubs
	.export _di
	.export _ei
	.export _irqrestore
	.export _swab
	.export outnewline
	.export outcharhex
	.export outstring
	.export _doexec

	.export interrupt_handler
	.export trap_handler

	.common

_di:
	stst	r1
	limi	IRQ_OFF
	rt
_ei:
	limi	IRQ_ON
	rt

; We mash the status bits - which is fine as we look like a function call
; and function calls don't preserve status
_irqrestore:
	lst	r4
	rt

outnewline:
	dect	r13
	mov	r11,*r13
	li	r0,0x0D00
	bl	@outchar
	li	r0,0x0A00
	bl	@outchar
	mov	*r13+,r11
	rt

outcharhex:
	dect	r13
	mov	r11,*r13
	dect	r13
	mov	r0, *r13
	dect	r13
	mov	r1, *r13
	dect	r13
	mov	r2, *r13
	swpb	r0
	mov	r0,r2
	mov	r2,r1
	srl	r1,4
	bl	@outhexdigit
	mov	r0,r1
	bl	@outhexdigit
	mov	*r13+,r2
	mov	*r13+,r1
	mov	*r13+,r0
	mov	*r13+,r11
	rt

outhexdigit:
	ai	r1,0x0F
	movb	@hextab(r1),r0
	b	@outchar
hextab:
	.ascii	"0123456789ABCDEF"

outstring:
	dect	r13
	mov	r11,*r13
outsl:
	movb	*r1+,r0
	jeq	@outstrend
	bl	@outchar
	jmp	@outsl
outstrend:
	mov	*r13+,r11
	rt


_swab:
	mov	r4,r1
	swpb	r1
	rt

;
;	Context handling. Quite ugly as we have to work via multiple
;	workspaces due to way the CPU is designed
;

;
;	Copy the urrent workspace setup (regs and R13-R15) onto the
;	stack for that workspace (r13 of it) and update the stack pointer
;
;
stack_workspace:
	li	r0,	16
	mov	@26(r13), r1
	ai	r1,	-36
	mov	r1, @26(r13)
save_next:
	mov	*r13+, *r1+
	dec	r0
	jne	@save_next
	mov	r14, *r1+
	mov	r15, *r1+
	rt
;
;	Return to a stacked workspace
;
pop_workspace:
	li	r0,	16
	mov	@26(r13), r1
pop_next:
	mov	*r1+, *r2+
	dec	r0
	jne	@pop_next
	mov	*r1+, r15
	mov	*r1+, r14
	mov	r1, @26(r13)
	rtwp

;
;	Check for signal. Running on IRQ path. Slightly different as our
;	workspace is already p
;
;
;	Check for signals. Running on kernel or irq workspace with user
;	memory mapped
;
deliver_signal:
	clr	r1
	movb	@_udata+U_DATA__U_CURSIG,r1
	jne	@is_signal
	; No signal - just go back to the old workspace
no_signal_race:
	rtwp
is_signal:
	swpb	r1
	mov	r1,r3
	a	r1,r1
	mov	@_udata+U_DATA__U_SIGVEC(r1),r2
	jeq	@no_signal_race
	clr	@_udata+U_DATA__U_SIGVEC(r1)
	movb	@__litb_0, @_udata + U_DATA__U_CURSIG
;
;	On entry r3 is the signal r2 is the vector
;
handle_signal:
	bl	@stack_workspace
	mov	@26(r13), r0
	dect	r0
	mov	r13, *r0		; save the workspace
	dect	r0
	mov	r3, *r0
	dect	r0
	li	r3, sigunwind
	mov	r1, *r0
	mov	r1,@26(r13)		; save the modified stack pointer
	mov	r2, r14
	rtwp				; into the handler
sigunwind:
	; We are in the user context
	dect	r13			; skip over signal number
	limi	IRQ_OFF
	mov	*r13+, r13		; get the old workspace
	b	@pop_workspace

;
;	We arrive here on our own new workspace.
;	R13 = workspace R14 = old PC R5 = old ST
;
;	In the old workspace R2 = syscall R4 = arg0 R5 = arg1
;
;	Some games required because R13 is also going to be our C stack
;
unix_syscall_entry:
	limi	IRQ_OFF
	mov	r13, r8		; remember the old workspace

	mov	@26(r8), r9	; user stack pointer


	mov	r9, @_udata+U_DATA__U_SYSCALL_SP
	movb	@4(r8), @_udata+U_DATA__U_CALLNO; user R2

	mov	r9,r0
	inct	r0		; over the return address

	li	r1,_udata + U_DATA__U_ARGN
	mov	@8(r8),*r1+	; 4 bytes from R4/R5
	mov	@10(r8),*r1+
	mov	*r0+, *r1+	; 4 bytes of argument block from stack
	mov	*r0+, *r1

	inc	@_udata+U_DATA__U_INSYS	; set to one

	;	Switch to the kernel stack
	;	old r13 is saved in r8 and C code will save that
	li	r13,kstack_top
	bl	@map_kernel_di
	limi	IRQ_ON
	bl	@_unix_syscall
	limi	IRQ_OFF
	clr	@_udata+U_DATA__U_INSYS
	bl	@map_proc_always

	;	Return stack (do we need this ?)
	mov	@_udata + U_DATA__U_SYSCALL_SP, r9

	;	Save the return values in the old context
	mov	@_udata+U_DATA__U_RETVAL,@2(r8)
	mov	@_udata+U_DATA__U_ERROR, *r8

	clr	r1
	movb	@_udata+U_DATA__U_CURSIG,r2
	jne	@is_signal
	limi	IRQ_ON
	mov	r8, r13
	rtwp

_doexec:
	bl	@map_proc_always
	mov	@_udata+U_DATA__U_ISP, r13
	clr	@_udata+U_DATA__U_INSYS
	;	Write r4 into the user workspace
	mov	r4,@user_ws+8
	mov	r13,@user_ws+26
	lwpi	user_ws
	clr	r0
	clr	r1
	clr	r2
	clr	r3
	clr	r5
	clr	r6
	clr	r7
	clr	r8
	clr	r9
	clr	r10
	clr	r11
	clr	r12
	clr	r14
	; Hack for now
	li	r14,0x0100	; FIXME program base
	clr	r15
	limi	IRQ_ON
	b	*r4

trap_handler:
	mov	@_udata + U_DATA__U_INSYS,r0
	jne	@trap_illegal
	mov	@_udata + U_DATA__U_ININTERRUPT,r0
	jne	@trap_illegal
	; We are in the wrong workspace for a syscall but this is going to
	; be short and terminal
	li	r13, kstack_top
	dect	r13
	li	r0, 9		; killed signal 9
	mov	r0, *r13
	b	@_doexit
	; Won't come back

trap_illegal:
	li r2,illegalmsg
trapx:
	bl @outstring
	bl @_plt_monitor
nmi_handler:
	bl @map_kernel_di
	li r2,nmimsg
	jmp @trapx

illegalmsg:
	.ascii '[illegal]'
	.byte 0
nmimsg:
	.ascii '[NMI]'
	.byte 0

;
;	This a bit more complicated. We took a free workspace change
;	when the interrupt was serviced. This also means we don't have
;	to save any registers on the fast path as such
;
interrupt_handler:
	; This is optimised for the no signal, no pre-emption
	mov	r13, r8
	; Assumes C environment doesn't mash r14/r15. It will nuke r13
	; so save it in r8
	;
	; Set up our C stack for the interrupt handler
	li	r13, istack_top
	bl	@map_save_kernel
	; Flag interrupt status
	li	r0,1
	mov	r0,@_udata+U_DATA__U_ININTERRUPT
	; Handle interrupt
	bl 	@_plt_interrupt
	; Restore R13 workspace pointer
	mov	r8, r13
	; Simple case - we were in the kernel
	mov	@_udata+U_DATA__U_INSYS,r0
	jne	@iretk
	; Pre-emption - the nasty case - handle it out of line
	movb	@_need_resched,r0
	jne	@preemption
	; No pre-emption, but may be signals pending. Map the user process
	clr	@_udata+U_DATA__U_ININTERRUPT
	bl	@map_proc_always
	b	@deliver_signal
	; Kernel fast path - restore the MMU instead of setting it for
	; the process. Again all the work is done by the work space flip
iretk:
	clr	@_udata+U_DATA__U_ININTERRUPT
	bl	@map_kernel_restore
	rtwp

;
;	We are on the interrupt register set and wp. That is itself in
;	onboard memory and will need to get saved
;
preemption:
	; Our workspace needs to end up stacked so we can recover it
	; after we've finished
	bl	@stack_workspace

	; Copy r13/r14/r15 into the workspace
	mov	r13, @26(r13)
	mov	r14, @28(r13)
	mov	r15, @30(r13)

	; Go back to the previous workspace (we've pushed a copy already)
	; and r13/14/15 are valid
	lwp	r13

	mov	r13, r8

	; Clear the flag
	movb	@__litb_0,@_need_resched

	; Get us into a state we can call kernel functions in C
	li	r13,kstack_top
	clr	@_udata+U_DATA__U_INSYS
	; Update our signal and run state
	bl	@_chksigs
	; restore workspace pointer
	mov	r8, r13
	mov	@_udata+U_DATA__U_PTAB,r1

	; If we were pre-empted running then set the punishment flag
	movb	@P_TAB__P_STATUS_OFFSET(r1),r0
	cb	@lit_P_RUNNING,r0
	jne	@not_running
	movb	@lit_P_READY,@P_TAB__P_STATUS_OFFSET(r1)
	movb	@P_TAB__P_FLAGS_OFFSET(r1),r0
	ori	r0, PFL_BATCH * 256
	movb	r0,@P_TAB__P_FLAGS_OFFSET(r1)
not_running:

	; and switch. This will stack the regs we care about on the
	; kstack of the task and save the needed pointers into the
	; process. When it returns it will have undone that so we will
	; be in a valid state
	bl	@_plt_switchout

	; recover the workspace pointer
	mov	r8, r13

	; We've resumed - could be from an interrupt completion, could
	; be from anything else. We know however we won't be in a syscall
	; or interrupt (as they are not pre-emptible)
	clr 	@_udata+U_DATA__U_ININTERRUPT
	clr	@_udata+U_DATA__U_INSYS
	bl	@map_proc_always
	; Pop the context we saved and return to it
	b	@pop_workspace

lit_P_RUNNING:
	.byte	P_RUNNING
lit_P_READY:
	.byte	P_READY

_set_cpu_type:
	rt

	.data

	.even

_sys_cpu:
	.byte	7
_sys_cpu_feat:
	.byte	1
_sys_stubs:
	.word	kernel_ws
	.word	unix_syscall_entry
	.ds	12

	.even
