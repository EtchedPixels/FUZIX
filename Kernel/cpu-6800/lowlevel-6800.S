;
;	Closely related to the 6803/6303 code but there are ABI differences
; 	and a lot less instructions we can use!
;
	.export _di
	.export _ei
	.export _irqrestore
	.export _swab
	.export _doexec
	.export _sys_cpu
	.export _sys_stubs
	.export _sys_cpu_feat
	.export _set_cpu_type
	.export unix_syscall_entry
	.export interrupt_handler
	.export nmi_handler
	.export trap_handler

	.export outnewline
	.export outcharhex
	.export outstring
	.export outx
	.export outd

	.export preemption

#include "../build/kernel.def"
#include "kernel-6800.def"

	.code

_di:
	tpa		; return cc codes in D
	sei
	rts

_ei:
	cli
	rts

_irqrestore:
	tsx
	ldaa	2,x 
			; D holds the return from di where A is the cc
	tap		; we trash overflow and carry but they are assumed
	jmp	__cleanup1

_swab:
	tsx
	ldab	2,x
	ldaa	3,x
	jmp	__cleanup2

	.common

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
	blt outh2
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
	stx @tmp
	ldab @tmp
	bsr outcharhex
	ldab @tmp+1
	bsr outcharhex
outsdone:
	rts

outd:
	psha
	pshb
	tab
	bsr outcharhex
	pulb
	pshb
	bsr outcharhex
	pulb
	pula
	rts

	

;
;	On enry our frame looks like this
;
;	12->	more arguments
;	11-10	first argument
;	9-8	return PC for caller to syscall
;	7-6	UserPC
;	5-4	X
;	3	A
;	2	B
;	1	CC
;
;
unix_syscall_entry:
	tsx
	sts _udata+U_DATA__U_SYSCALL_SP
	staa _udata+U_DATA__U_CALLNO
	ldaa 9,x
	ldab 10,x
	staa _udata+U_DATA__U_ARGN
	stab _udata+U_DATA__U_ARGN+1
	ldaa 11,x
	ldab 12,x
	staa _udata+U_DATA__U_ARGN+2
	stab _udata+U_DATA__U_ARGN+3
	ldaa 13,x
	ldab 14,x
	staa _udata+U_DATA__U_ARGN+4
	stab _udata+U_DATA__U_ARGN+5
	ldaa 15,x
	ldab 16,x
	staa _udata+U_DATA__U_ARGN+6
	stab _udata+U_DATA__U_ARGN+7
	ldaa #1
	staa _udata+U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	lds #kstack_top
	jsr map_kernel_di		; no-op in pure banked
	cli
	jsr _unix_syscall
	sei
	clr _udata+U_DATA__U_INSYS
	jsr map_proc_always	; no-op in pure banked
	lds _udata+U_DATA__U_SYSCALL_SP
	; Now the fun bit - the CPU saves X and D so we have to patch them
	; into the return frame
	ldaa _udata+U_DATA__U_RETVAL
	ldab _udata+U_DATA__U_RETVAL+1
	tsx
	stab 1,x			; return in D
	staa 2,x
	ldaa _udata+U_DATA__U_ERROR
	ldab _udata+U_DATA__U_ERROR+1
	staa 3,x			; error in X
	stab 4,x
rti_return:
	ldab _udata+U_DATA__U_CURSIG
	beq no_signal
	clr _udata+U_DATA__U_CURSIG
	pshb
	lslb
	ldaa #<_udata+U_DATA__U_SIGVEC
	aba
	ldaa #>_udata+U_DATA__U_SIGVEC
	adca #0
	stab @tmp+1
	staa @tmp
	ldx @tmp
	ldaa 0,x
	ldab 1,x
	beq no_signal_pop
	clr 0,x
	clr 1,x
	; Save the vector
	staa @tmp
	stab @tmp+1
	; Argument to signal handler
	; First half is already on the stack
	; Push a 0 byte to size it for C
	pulb		; Get signal back into B
	; Now push the vector
	ldaa @tmp+1
	psha
	ldaa @tmp
	psha
	ldx  PROGLOAD+16	; Helper vector (FIXME: assumes binary relocatd here)
	; We are off the kernel stack at this point so we can
	; enable interrupts and leap into userspace
	cli
	; Signal as 16 bit value
	clra
	; Into helper
	jsr ,x
	ins	; Drop the vector
no_signal_pop:
	ins
no_signal:
	rti	; Unwind the original frame (containg the
		; errno and return values ready)

_doexec:
	tsx
	ldx 2,x
	sei
	jsr map_proc_always
	lds _udata+U_DATA__U_ISP
	des				; offset by 1 on the 680X
	clr _udata+U_DATA__U_INSYS
	cli
	stx @tmp			; X & 0xFF00 is our base
	ldaa @tmp
	clrb
	jmp 0,x

trap_handler:
	tst _udata + U_DATA__U_INSYS
	bne trap_illegal
	tst _udata + U_DATA__U_ININTERRUPT
	bne trap_illegal
	clra
	ldab #9
	pshb
	psha
	ldx _udata+U_DATA__U_PTAB
	ldab P_TAB__P_PID_OFFSET,x
	ldaa P_TAB__P_PID_OFFSET+1,x
	pshb
	psha
	ldaa #39
	ldab #04	; kill with 2 args
	swi		; Will never return

trap_illegal:
	ldx #illegalmsg
trapx:
	jsr outstring
	jsr _plt_monitor

nmi_handler:
	jsr map_kernel_di
	ldx #nmimsg
	bra trapx

illegalmsg:
	.ascii '[illegal]'
	.byte 0
nmimsg:
	.ascii '[NMI]'
	.byte 0
;
;	The trap saved our entire register state for us
;
interrupt_handler:
	sts istack_switched_sp
	lds #istack_top
	jsr map_save_kernel

	ldaa #1
	staa _udata+U_DATA__U_ININTERRUPT
	; Save the C direct page values so we can re-enter
	; If we turn on register variables we will need to save them too
	ldaa @tmp
	psha
	ldaa @tmp+1
	psha
	ldaa @tmp1
	psha
	ldaa @tmp1+1
	psha
	ldaa @tmp2
	psha
	ldaa @tmp2+1
	psha
	ldaa @hireg
	psha
	ldaa @hireg+1
	psha
	ldaa @tmp3
	psha
	ldaa @tmp3+1
	psha
	ldaa @tmp4
	psha
	ldaa @tmp4+1
	psha
	ldaa @tmp5
	psha
	ldaa @tmp5+1
	psha
	jsr _plt_interrupt
	; Restore the C direct page
	pula
	staa @tmp5+1
	pula
	staa @tmp5
	pula
	staa @tmp4+1
	pula
	staa @tmp4
	pula
	staa @tmp3+1
	pula
	staa @tmp3
	pula
	staa @hireg+1
	pula
	staa @hireg
	pula
	staa @tmp2+1
	pula
	staa @tmp2
	pula
	staa @tmp1+1
	pula
	staa @tmp1
	pula
	staa @tmp+1
	pula
	staa @tmp
	; If we interrupted kernel mode then we just put the map back and
	; return
	tst _udata+U_DATA__U_INSYS
	bne iretk
	; If we had to reschedule it's more complicated
	tst _need_resched
	bne preemption
	; Put the process memory map back
	jsr map_proc_always
	; Put the process stack back
	lds istack_switched_sp
	; We are no longer on the istack or in an interrupt
	clr _udata+U_DATA__U_ININTERRUPT
	; Don't deliver signals if we were in a system call
	tst _udata+U_DATA__U_INSYS
	bne no_signals
	; We may have a signal to deliver.so exit via that path
	jmp rti_return
	; Nothing special happened so just rti to get the CPU state
	; back and continue life
no_signals:
	rti
	;
	; Return to kernel mode
	; We don't check signals in this case and we use map_restore
	;
iretk:
	jsr map_restore
	lds istack_switched_sp
	clr _udata+U_DATA__U_ININTERRUPT
	rti

;
;	Task switch due to an interrupt. Tricky in places on a 6800
;
preemption:
	; Acknowledge the request
	clr _need_resched
	; Copy the user stack save to the syscall save
	ldaa istack_switched_sp
	staa _udata+U_DATA__U_SYSCALL_SP
	ldaa istack_switched_sp+1
	staa _udata+U_DATA__U_SYSCALL_SP+1
	; Move onto our process kernel stack
	; We can't stay on the istack as it'll be gone when
	; we return.
	lds #kstack_top
	; Make it look like we are in a syscall
	ldab #1
	stab _udata+U_DATA__U_INSYS
	; Update our signals
	jsr _chksigs
	; Correct our process state
	ldx _udata+U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	; Set the punishment flag if we used our time up
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
	ldaa P_TAB__P_FLAGS_OFFSET,x
	orab #PFL_BATCH
	stab P_TAB__P_FLAGS_OFFSET,x
	;
	; At this point we are on the kernel stack and it all
	; looks like a syscall. We can now switch out. The
	; interrupt is effectively over and the new task will
	; set the flags accordingly. In time we will pop out of
	; plt_switchout again as if we had finished a syscall
	;
not_running:
	jsr _plt_switchout
	; We no longer want interrupts off when we run
	clr _udata+U_DATA__U_ININTERRUPT
	; We are not in a syscall
	clr _udata+U_DATA__U_INSYS
	; We are mapoped in memory
	jsr map_proc_always
	; Move to user stack before interrupt
	lds _udata+U_DATA__U_SYSCALL_SP
	ldaa _udata+U_DATA__U_CURSIG
	beq no_signals
	; Do the signal return path
	jmp rti_return

	.code

;
;	FIXME: eventually test 6803 v 6303 and for 6303
;	set features to 3 (xgdx as well as 6803) 
;

	.setcpu 6303

;
;	This a 6800 spec
;
_set_cpu_type:
	ldx #0
	sec
	cpx #0		; on 6800 does not change carry
			; on 6803 it does
	bcs is_6800
	clrb
	ldx #1
	xgdx		; appears to do nothing on the 6803
	tstb
	beq is_6803
	ldab #3		; 6303 feature set also present
	bra setfeat
is_6803:
	ldab #1
setfeat:
	stab _sys_cpu_feat
is_6800:
	rts


	.setcpu 6800

	.data
_sys_cpu:
	.byte 2		; 6800 class CPU
_sys_cpu_feat:
	.byte 0		; 6800
_sys_stubs:
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

;
;	Support routines
;


	.code
	.export _strlen

	.setcpu 6800

_strlen:
	tsx
	ldx 2,x
	clra
	clrb
cl:	tst ,x
	beq to_rts
	inx
	addb #1
	adca #0
	bra cl
to_rts:
	jmp __cleanup2


	.export _memset

	.code

_memset:
	tsx
	ldab	5,x
	stab	@tmp		; pattern
	ldaa	6,x		; length
	ldab	7,x
	ldx	2,x		; destination
	bsr	snextblock
	tsx
	ldaa	2,x
	ldab	3,x
	jmp	__cleanup6

snextblock:
	tsta
	beq	tailset
	; Set 256 bytes
	pshb
	psha
	clrb
	bsr	tailset
	pula
	pulb
	deca
	bra	snextblock

tailset:
	ldaa	@tmp
clearloop:
	staa	,x
	inx
	decb
	bne	clearloop
	rts

;
;	There isn't a nice way to do this on 680x/630x.
;
;
	.export _memcpy

	.setcpu 6800
	.code

_memcpy:
	tsx
	ldaa	2,x
	ldab	3,x
	staa	@tmp		; destination
	stab	@tmp+1
	ldaa	6,x		; length
	oraa	7,x
	beq	nocopy
	ldaa	6,x
	ldab	7,x
	ldx	4,x		; src
	bsr	nextblock
	tsx
nocopy:
	ldaa	2,x
	ldab	3,x
	jmp	__cleanup6

nextblock:
	tsta
	beq	tailcopy
	; Copy 256 bytes repeatedly until we get to the leftovers
	pshb
	psha
	clrb
	bsr	tailcopy
	pula
	pulb
	deca
	bra	nextblock

tailcopy:
	ldaa	,x
	inx
	stx	@tmp2
	ldx	@tmp
	staa	,x
	inx
	stx	@tmp
	ldx	@tmp2
	decb
	bne	tailcopy
	rts
