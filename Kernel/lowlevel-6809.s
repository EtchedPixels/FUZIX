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
	.globl	_lshrhi3
	.globl	___ashlsi3
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
	.globl _kernel_flag
	.globl _inint
	.globl _platform_interrupt

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

        ; imported symbols
        .globl _trap_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
        .globl dispatch_process_signal
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
; The lack of shared RAM leads to the rather ugly SAM_ macros needing to
; be here for SAM based boxes as we have no valid stack at this point
;
unix_syscall_entry:
	SAM_KERNEL	; stack now invalid
	; make sure the interrupt logic knows we are in kernel mode
	lda #1
	sta _kernel_flag

	leax 14,s	; 12 stacked by the swi + return address of caller
	ldy #U_DATA__U_ARGN
	SAM_USER
	ldd 4,s		; first argument in swi stacked X
	SAM_KERNEL
	std ,y++
	SAM_USER
	ldd ,x++	; second argument from caller's stack
	SAM_KERNEL
	std ,y++
	SAM_USER
	ldd ,x++ 	; third
	SAM_KERNEL
	std ,y++
	SAM_USER
	ldd ,x++ 	; fourth
	SAM_KERNEL
	std ,y++
	SAM_USER
	ldd 1,s		; stacked D register -> syscall number
	SAM_KERNEL
	stb U_DATA__U_CALLNO
        ; save process stack pointer (in user page)
        sts U_DATA__U_SYSCALL_SP
        ; switch to kernel stack (makes our stack valid again)
        lds #kstack_top

        ; map in kernel keeping common
	jsr map_kernel

        ; re-enable interrupts
        andcc #0xef

        ; now pass control to C
        jsr _unix_syscall

        orcc #0x10
	; let the interrupt logic know we are not in kernel mode any more
	; kernel_flag is not in common so write it before we map it away
	clr _kernel_flag

        ; map process memory back in based on common (common may have
        ; changed on a task switch)
        jsr map_process_always

        ; switch back to user stack
        lds U_DATA__U_SYSCALL_SP
	; stack is no longer valid

	SAM_USER
	; stack is now valid but user stack
        ; check for signals, call the handlers
        jsr dispatch_process_signal

	SAM_KERNEL
        ; check if error condition to be signalled on return
        ldd U_DATA__U_ERROR
	beq not_error
	ldx #-1	
        ; error code in d, result in x
        bra unix_return

not_error:
        ; no error to signal! return syscall return value instead of error code
        ldx U_DATA__U_RETVAL
	ldd #0		; just for setting Z flag
unix_return:
	; we never make a syscall from in kernel space
	SAM_USER
	stx 4,s		; replace stacked values before rti
	std 1,s
	tfr cc,a	; Z determined by D
	sta ,s		; replace stacked CC to signal error
	rti

;
;	In SAM land this function is called on the user stack with the user
;	memory mapped, we must rts on the right bank !
;
dispatch_process_signal:
        ; check if any signal outstanding
	SAM_KERNEL
        ldb U_DATA__U_CURSIG
        beq dosigrts
        ; put number in X as the argument for the signal handler
	; so extend it to 16bit
	clra
	tfr d,x

	lslb		;	2 bytes per entry
        ; load the address of signal handler function
	ldy #U_DATA__U_SIGVEC
	ldu b,y		; now u = udata.u_sigvec[cursig]

        ; udata.u_cursig = 0;
	clr U_DATA__U_CURSIG

        ; restore signal handler to the default.
        ; udata.u_sigvec[cursig] = SIG_DFL;
        ; SIG_DFL = 0
	leay b,y
	clr ,y+
	clr ,y

        ldy #signal_return
	SAM_USER
        pshs y      ; push return address

        andcc #0xef
	jmp ,u

signal_return:
        orcc #0x10
;
;	FIXME: port over the Z80 loop and check for next signal
;
dosigrts:
	; right stack for the rts instruction
	SAM_USER
        rts

_doexec:
	; x is the jump address
        orcc #0x10
	; this is a funny extra path out of syscall so we must also cover
	; the exit from kernel here
	clr _kernel_flag

        ; map task into address space (kernel_flag is no longer mapped, don't
	; re-order this)
	; preserves x
        jsr map_process_always

        ; u_data.u_insys = false
        clr U_DATA__U_INSYS
	; At this point the stack goes invalid
        lds U_DATA__U_ISP
	andcc #0xef			; IRQs on
	SAM_USER			; Stack valid, go to user code
        jmp ,x

;
;	Very simple IRQ handler, we get interrupts and we may have to
;	poll ttys from it. The more logic we could move to common here the
;	better.
;
;	For SAM based machines this involves some craziness as we have no
;	common inter-bank RAM. In the SAM case we will always enter with
;	the kernel mapped, but our stack on entry is likely to be invalid
;
;	Thus we have to have inlined SAM conditionals - bletch
;
;	Y is used for the SAM logic so preserve it always
;
interrupt_handler:
	    SAM_SAVE		; loads Y
	    SAM_KERNEL

	    ; Do not use the stack before the switch...

	    ; FIXME: add profil support here (need to keep profil ptrs
	    ; unbanked if so ?)

            ; don't allow us to run re-entrant, we've only got one interrupt stack
	    lda U_DATA__U_ININTERRUPT
            bne interrupt_return
            inca
            sta U_DATA__U_ININTERRUPT

            ; switch stacks
            sts istack_switched_sp
	    ; Unlike Z80 even the istack is banked, it's only valid on SAM
	    ; based boxes when SAM_KERNEL is true
            lds #istack_top-2
	    ; FIXME: check store/dec order might not need to be -2 here!!!!

	    jsr map_save

	    SAM_USER
	    lda 0		; save address 0 contents for checking
	    SAM_KERNEL

	    ; preserves registers
	    jsr map_kernel
	    ;
	    ; kernel_flag is in the kernel map so we need to map early, we
	    ; need to map anyway for trap_signal
	    ;
	    ldb _kernel_flag
	    pshs b,cc
	    bne in_kernel

            ; we're not in kernel mode, check for signals and fault
            cmpa #0x7E		; JMP at 0
	    beq nofault
	    jsr map_process_always ; map the process
	    lda #0x7E		; put it back
	    sta 0		; write
	    jsr map_kernel	; restore the map
	    ldx #11		; SIGSEGV
	    jsr trap_signal	; signal the user with a fault

nofault:
in_kernel:
            ; set inint to true
            lda #1
            sta _inint

	    ; this may task switch if not within a system call
	    ; if we switch then istack_switched_sp is out of date.
	    ; When this occurs we will exit via the resume path 
	    ;
	    ; Y is caller saves so we'll get the right Y back
	    ; for our SAM_RESTORE
            jsr _platform_interrupt

            clr _inint

	    puls b,cc			; Z = in kernel
	    bne in_kernel_2

	    ; On a return to user space always do a new map, we may have
	    ; changed process
	    jsr map_process_always
            bra int_switch
	    ; On a return from an interrupt in kernel mode restore the old
	    ; mapping as it will vary during kernel activity and the kernel
	    ; wants it put back as it was before
in_kernel_2:
	    jsr map_restore
int_switch:
            lds istack_switched_sp	; stack back
            clr U_DATA__U_ININTERRUPT
            lda U_DATA__U_INSYS
            bne interrupt_return

	    SAM_USER
            ; we're not in kernel mode, check for signals
	    ; runs off the user stack so need user ram mapped on a SAM
	    ; based platform
	    pshs y
            jsr dispatch_process_signal
	    puls y

interrupt_return:
	    SAM_RESTORE			; uses the saved Y
            rti

;  Enter with X being the signal to send ourself
trap_signal:
	    pshs x
	    ldx U_DATA__U_PTAB	;  ssig(pid, X)
            jsr _ssig
	    puls x,pc

;  Called from process context (hopefully)
null_handler:
	    SAM_KERNEL
	    ; kernel jump to NULL is bad
	    lda U_DATA__U_INSYS
	    beq trap_illegal
	    SAM_USER
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
	    jsr _trap_monitor

dpsmsg:	    .ascii "[dispsig]"
            .db 13,10,0


nmimsg:     .ascii "[NMI]"
            .db 13,10,0

nmi_handler:
	SAM_KERNEL
	lds #istack_top - 2		; We aren't coming back so this is ok
	jsr map_kernel
        ldx #nmimsg
	jsr outstring
        jsr _trap_monitor


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; outstring: Print the string at X until 0 byte is found
; destroys: AF HL
outstring:
	lda ,x+
	beq outstrdone
        jsr outchar
        bra outstring
outstrdone:
	rts

; print the string at (HL) in hex (continues until 0 byte seen)
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

outd:  ; prints D in hex.
	pshs b
        tfr b,a
        jsr outcharhex
        puls b
        jsr outcharhex
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

; print the byte in A as a two-character hex value
outcharhex:
	pshs a
        lsra
        lsra
        lsra
        lsra
        jsr outnibble
	puls a
        jsr outnibble
        rts

; print the nibble in the low four bits of A
outnibble:
        anda #0x0f ; mask off low four bits
        cmpa #9
        ble numeral ; less than 10?
        adda #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:adda #0x30 ; start at '0' (0x30='0')
        jsr outchar
        rts


div0:
	ldx	#div0msg
	jsr	outstring
	jsr	_trap_monitor
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

___ashlsi3:
	pshs	u
	cmpb	#16
	blt	try8
	subb	#16
	; Shift by 16
	ldu	2,x
	stu	,x
try8:
	cmpb	#8
	blt	try_rest
	subb	#8
	; Shift by 8

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
