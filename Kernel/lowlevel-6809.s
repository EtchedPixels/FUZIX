;
;	Common elements of low level interrupt and other handling. We
; collect this here to minimise the amount of platform specific gloop
; involved in a port
;
;	Based upon code (C) 2013 William R Sowerbutts
;

	.module lowlevel

	; compiler support
	.globl	_euclid
	.globl	_udivhi3
	.globl	_umodhi3
	.globl  _mulhi3
	.globl	_ashlhi3
	.globl	_ashrhi3
	.globl	_lshrhi3
	.globl	___ashlsi3
	.globl  ___ashrsi3
	.globl	_swab

	; debugging aids
	.globl outcharhex
	.globl outd,outx,outy
	.globl outnewline
	.globl outstring
	.globl outstringhex
	.globl outnibble

	; platform provided functions
	.globl map_kernel
	.globl map_process_always
        .globl map_save
        .globl map_restore
	.globl outchar
	.globl _need_resched
	.globl _plt_interrupt

        ; exported symbols
        .globl unix_syscall_entry
	.globl null_handler
	.globl _system_tick_counter
	.globl unix_syscall_entry
	.globl dispatch_process_signal
        .globl _doexec
        .globl trap_illegal
	.globl nmi_handler
	.globl interrupt_handler
	.globl _sys_cpu
	.globl _sys_cpu_feat
	.globl _sys_stubs
	.globl _set_cpu_type

        ; imported symbols
        .globl _plt_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
	.globl istack_switched_sp
	.globl istack_top
	.globl _ssig

        include "platform/kernel.def"
        include "kernel09.def"

        .area .common

; entry point for Fuzix system calls
;
; Called by swi, which has already saved our CPU state for us
;
unix_syscall_entry:
	leax 14,s	; 12 stacked by the swi + return address of caller
	ldy #U_DATA__U_ARGN
	ldd 4,s		; first argument in swi stacked X
	std ,y++
	ldd ,x++	; second argument from caller's stack
	std ,y++
	ldd ,x++ 	; third
	std ,y++
	ldd ,x++ 	; fourth
	std ,y++
	ldd 1,s		; stacked D register -> syscall number
	stb U_DATA__U_CALLNO
        ; save process stack pointer (in user page)
        sts U_DATA__U_SYSCALL_SP
        ; switch to kernel stack (makes our stack valid again)
        lds #kstack_top

	; we are in syscall state
	lda #1
	sta U_DATA__U_INSYS

        ; map in kernel keeping common
	jsr map_kernel

        ; re-enable interrupts
        andcc #0xef

        ; now pass control to C
        jsr _unix_syscall

        orcc #0x10
	; let the interrupt logic know we are not in kernel mode any more
	clr U_DATA__U_INSYS

        ; map process memory back in based on common (common may have
        ; changed on a task switch)
        jsr map_process_always

        ; switch back to user stack
        lds U_DATA__U_SYSCALL_SP

	; stack is now valid but user stack
        ; check for signals, call the handlers
        jsr dispatch_process_signal

        ; check if error condition to be signalled on return
        ldd U_DATA__U_ERROR
	beq not_error
	ldx #-1	
        ; error code in d, result in x
        bra unix_return

not_error:
        ; no error to signal! return syscall return value instead of error code
        ldx U_DATA__U_RETVAL
unix_return:
	; we never make a syscall from in kernel space
	stx 4,s		; replace stacked values before rti
	std 1,s
	rti
;
;	We must rts on the right bank !
;
dispatch_process_signal:
        ; check if any signal outstanding
        ldb U_DATA__U_CURSIG
        beq dosigrts

	; The signal handler is entitled to make syscalls which will
	; in turn trash these two
	ldx U_DATA__U_RETVAL
	ldy U_DATA__U_ERROR
	pshs x,y
        ; put number in X as the argument for the signal handler
	; so extend it to 16bit
	clra
	tfr d,x

	lslb		;	2 bytes per entry
        ; load the address of signal handler function
	ldy #U_DATA__U_SIGVEC
	leay b,y
	ldu ,y		; now u = udata.u_sigvec[cursig]

        ; udata.u_cursig = 0;
	clr U_DATA__U_CURSIG

        ; restore signal handler to the default.
        ; udata.u_sigvec[cursig] = SIG_DFL;
        ; SIG_DFL = 0
	clr ,y+
	clr ,y

        andcc #0xef
	jsr ,u

signal_return:
        orcc #0x10
	puls x,y
	stx U_DATA__U_RETVAL
	sty U_DATA__U_ERROR
dosigrts:
        rts

_doexec:
	; x is the jump address
        orcc #0x10
	; this is a funny extra path out of syscall so we must also cover
	; the exit from kernel here

	; map task into address space
	; preserves x
        jsr map_process_always

	; base address
	ldy U_DATA__U_CODEBASE
        ; u_data.u_insys = false
        clr U_DATA__U_INSYS
	; At this point the stack goes invalid
        lds U_DATA__U_ISP
	andcc #0xef			; IRQs on
        jmp ,x

;
;	Very simple IRQ handler, we get interrupts and we may have to
;	poll ttys from it. The more logic we could move to common here the
;	better.
;


;
;	Called when interrupts have been re-enabled within the timer
;	interrupt. We hand it to the platform re-interrupt handler. If
;	none is expected then it can panic, or if the platform is clever
;	it can do the needed work.
;
reinterrupt:
	jsr _plt_reinterrupt
	; Signals and other magic will happen when the first level of
	; interrupt handling returns
	rti

interrupt_handler:
	; If the platform interrupt code re-enabled interrupts then
	; we are on the interrupt stack already and platform author
	; is assumed to know what they are doing 8)
	tst U_DATA__U_ININTERRUPT
	bne reinterrupt

	; Do not use the stack before the switch...
	; FIXME: add profil support here (need to keep profil ptrs
	; unbanked if so ?)

	lda #1
        sta U_DATA__U_ININTERRUPT

        ; switch stacks
        sts istack_switched_sp
        lds #istack_top-2
	; FIXME: check store/dec order might not need to be -2 here!!!!

	jsr map_save

	ldb U_DATA__U_INSYS	; In a system call ?
	bne in_kernel

        ; we're not in kernel mode, check for signals and fault
	lda 0		; save address 0 contents for checking
        cmpa #0x7E		; JMP at 0
	beq nofault
	lda #0x7E		; put it back
	sta 0		; write
	jsr map_kernel
	ldd #11		; SIGSEGV
	jsr trap_signal	; signal the user with a fault

nofault:
in_kernel:
        jsr map_kernel

	;
	; If the kernel decides to task switch it will set
	; _need_resched, and will only do so if the caller was in
	; user space so has a free kernel stack

        jsr _plt_interrupt

        ldx istack_switched_sp	; stack back
        clr U_DATA__U_ININTERRUPT
        lda U_DATA__U_INSYS
        bne interrupt_return_x
	lda _need_resched
	beq no_switch

	clr _need_resched
	stx U_DATA__U_SYSCALL_SP	; save again somewhere safe for
					; preemption
	; Pre emption occurs on the task stack. Conceptually its a
	; not quite a syscall
	lds #kstack_top
	jsr _chksigs		; check signal state
	;
	ldx U_DATA__U_PTAB
	; Move to ready state
	lda P_TAB__P_STATUS_OFFSET,x
	cmpa #P_RUNNING
	bne not_running
	lda #P_READY
	sta P_TAB__P_STATUS_OFFSET,x
not_running:
	; Sleep on the kernel stack, IRQs will get re-enabled if need
	; be
	jsr _plt_switchout
	;
	; We will resume here after the pre-emption. Get back onto
	; the user stack and map ourself in
	jsr map_process_always
	lds U_DATA__U_SYSCALL_SP
	bra intdone

	    ; Not task switching - the easy and usual path
no_switch:   
	; On a return from an interrupt restore the old mapping as it
	; will vary during kernel activity and we need to put it put
	; it back as it was before the interrupt
	; pre-emption is handled differently...
	jsr map_restore
	lds istack_switched_sp

intdone: 
        ; we're not in kernel mode, check for signals
	; runs off the user stack
	pshs y
        jsr dispatch_process_signal
	puls y

interrupt_return:
        rti
	;	From kernel
	;	Restore the user mapping then
	;	switch to the user stack ptr
interrupt_return_x:
	jsr map_restore
	tfr x,s
	bra interrupt_return

;  Enter with X being the signal to send ourself
trap_signal:
	pshs x
	ldx U_DATA__U_PTAB	;  ssig(pid, X)
        jsr _ssig
	puls x,pc

;  Called from process context (hopefully)
null_handler:
	; kernel jump to NULL is bad
	lda U_DATA__U_INSYS
	beq trap_illegal
	; user is merely not good
	; check order of push arguments !!
        ldx #7
	ldy U_DATA__U_PTAB
	ldd #10		;	signal (getpid(), SIGBUS)
	pshs d,y
	swi
	puls d,y
	ldd #0
	tfr d,x
	pshs d,x
        swi			; exit

illegalmsg: .ascii "[trap_illegal]"
        .db 13,10,0

trap_illegal:
	ldx #illegalmsg
	jsr outstring
	jsr _plt_monitor

dpsmsg:	.ascii "[dispsig]"
        .db 13,10,0


nmimsg: .ascii "[NMI]"
        .db 13,10,0

nmi_handler:
	lds #istack_top-2		; We aren't coming back so this is ok
	jsr map_kernel
        ldx #nmimsg
	jsr outstring
        jsr _plt_monitor

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	CPU type management
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We don't use the stubs as we have a proper architected syscalling
; interface via swi. Set it here so the binary gets 16bytes of free
; but uninteresting noise
_sys_stubs:

	.area .commondata

_sys_cpu:
	.byte 4
_sys_cpu_feat:
	.byte 0

	.area .common
;
;	Check for a 6309 (as per The 6309 Book)
;
_set_cpu_type:
	pshs d
	.dw 0x1043
	cmpb 1,s
	puls d
	beq is8
	lda #1
	sta _sys_cpu_feat
is8:
	rts


; outstring: Print the string at X until 0 byte is found
; destroys: A, X
outstring:
	lda ,x+
	beq outstrdone
        jsr outchar
        bra outstring
outstrdone:
	rts

; print the string at (X) in hex (continues until 0 byte seen)
outstringhex:
	lda ,x+
	beq outstrdone
        jsr outcharhex
	lda #32
	jsr outchar
        bra outstringhex

; output a newline
outnewline:
        lda #0x0d  ; output newline
        jsr outchar
        lda #0x0a
        jsr outchar
        rts

outx:  ; prints X
	pshs d
	tfr x,d
	bsr outd
	puls d,pc

outy:  ; prints Y
	pshs d
	tfr y,d
	bsr outd
	puls d,pc

outd:  ; prints D in hex.
	pshs b
	bsr outcharhex
	puls a
	; FALL THROUGH

; print the byte in A as a two-character hex value
outcharhex:
	pshs a
        lsra
        lsra
        lsra
        lsra
        bsr outnibble
	puls a
	; FALL THROUGH

; print the nibble in the low four bits of A
outnibble:
        anda #0x0f ; mask off low four bits
        cmpa #9
        ble num    ; less than 10?
        adda #0x07 ; start at 'A' (10+7+0x30=0x41='A')
num:    adda #0x30 ; start at '0' (0x30='0')
        jsr outchar
        rts


div0:
	ldx	#div0msg
	jsr	outstring
	jsr	_plt_monitor
div0msg	.ascii	'Divby0'
	.db	13,10,0
;
;	Maths helpers - could be called from anywhere in C code
;	From the GCC support code (except for swab)
;
_umodhi3:
	ldd	2,s
	beq	div0
	pshs	x
	jsr	_euclid
	leas	2,s
	tfr	d,x
	rts

_udivhi3:
	ldd	2,s
	beq	div0
	pshs	x
	jsr	_euclid
	puls	x,pc

	left=5
	right=1			; word
	count=0			; byte
	CARRY=1			; alias
_euclid:
	leas	-3,s		; 2 local variables
	clr	count,s		; prescale divisor
	inc	count,s
	tsta
presc:
	bmi	presc_done
	inc	count,s
	aslb
	rola
	bra	presc
presc_done:
	std	right,s
	ldd	left,s
	clr	left,s		; quotient = 0
	clr	left+1,s
mod1:
	subd	right,s		; check subtract
	bcc	mod2
	addd	right,s
	andcc	#~CARRY
	bra	mod3
mod2:
	orcc	#CARRY
mod3:
	rol	left+1,s	; roll in carry
	rol	left,s
	lsr	right,s
	ror	right+1,s
	dec	count,s
	bne	mod1
	leas	3,s
	rts

_mulhi3:
	pshs	x
	lda   5,s   ; left msb * right lsb * 256
	ldb   ,s
	mul
	tfr   b,a
	clrb
	tfr   d,x
	ldb   1,s   ; left lsb * right msb * 256
	lda   4,s
	mul
	tfr   b,a
	clrb
	leax  d,x
	ldb   1,s   ; left lsb * right lsb
	lda   5,s
	mul
	leax  d,x
	puls	d,pc  ; kill D to remove initial push

_swab:
	exg x,d		; into accumulator
	exg a,b		; swap bytes over
	exg d,x		; back into result
	rts

_ashlhi3:
	pshs	x
_ashlhi3_1:
	leax	-1,x
	cmpx	#-1
	beq	_ashlhi3_2
	aslb
	rola
	bra	_ashlhi3_1
_ashlhi3_2:
	puls	x,pc



___ashrsi3:
	pshs	u
	; FIXME temporary hack until we fix gcc-6809 or our use of it
	; the argument passing doesn't match so we'll mangle it
	ldu 4,s
	stu ,x
	ldu 6,s
	stu 2,x
	ldb 9,s
	;; FIXME: insert 16 optimization here
	;; remember to propagate top bit for signage
try_8@	cmpb	#8
	blo 	try_rest@
	subb	#8
	ldu	1,x		; shift what we can down by 1 byte
	stu	2,x
	lda	,x
	sta	1,x
	clr	,x		; default top byte to positive
	tst	1,x		; test old msb for sign
	bpl     try_8@		; go try another 8 shifts
	dec	,x		; dec to make top byte negative
	bra	try_8@		; go try another 8 shifts
try_rest@
	tstb		
	beq	done@
do_rest@
	; Shift by 1
	asr	,x
	ror	1,x
	ror	2,x
	ror	3,x
	decb
	bne	do_rest@
done@
	puls	u,pc

	
___ashlsi3:
	pshs	u

	; FIXME temporary hack until we fix gcc-6809 or our use of it
	; the argument passing doesn't match so we'll mangle it
	ldu 4,s
	stu ,x
	ldu 6,s
	stu 2,x
	ldb 9,s

	cmpb	#16
	blt	try8
	subb	#16
	; Shift by 16
	ldu	2,x
	stu	,x
	ldu	#0
	stu	2,x
try8:
	cmpb	#8
	blt	try_rest
	subb	#8
	; Shift by 8
	ldu	1,x
	stu	,x
	lda	3,x
	sta	2,x
	clr	3,x

try_rest:
	tstb
	beq	done
do_rest:
	; Shift by 1
	asl	3,x
	rol	2,x
	rol	1,x
	rol	,x
	decb
	bne	do_rest
done:
	puls	u,pc

_lshrhi3:
	pshs	x
_lshrhi3_1:
	leax	-1,x
	cmpx	#-1
	beq	_lshrhi3_2
	lsra
	rorb
	bra	_lshrhi3_1
_lshrhi3_2:
	puls	x,pc

_ashrhi3:
	pshs	x
1$:
	leax	-1,x
	cmpx	#-1
	beq	2$
	asra
	rorb
	bra	1$
2$:
	puls	x,pc
