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

        .area _COMMONMEM

; entry point for UZI system calls
;
; Called by swi, which has already saved our CPU state for us
;
unix_syscall_entry:
        orcc #0x10		; interrupts off
	; FIXME: save the zero page ptr ??

	; make sure the interrupt logic knows we are in kernel mode
	lda #1
	sta _kernel_flag

;FIXME
;
; locate function call arguments on the userspace stack
;        ld hl, #18     ; 16 bytes machine state, plus 2 bytes return address
;        add hl, sp
;        ; save system call number
;        ld a, (hl)
;        ld (U_DATA__U_CALLNO), a
;        ; advance to syscall arguments
;        inc hl
;        inc hl
;        ; copy arguments to common memory
;        ld bc, #8      ; four 16-bit values
;        ld de, #U_DATA__U_ARGN
;        ldir           ; copy

        ; save process stack pointer
        sts U_DATA__U_SYSCALL_SP
        ; switch to kernel stack
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

        ; check for signals, call the handlers
        jsr dispatch_process_signal

        ; check if error condition to be signalled on return
        ldd U_DATA__U_ERROR
	ldx #-1	
        ; error code in d, result in x
        bra unix_return

not_error:
        ; no error to signal! return syscall return value instead of error code
        ldx U_DATA__U_RETVAL
	ldd #0
unix_return:
	; zero page restore ??
        andcc #0xef
	rti

dispatch_process_signal:
        ; check if any signal outstanding
        ldb U_DATA__U_CURSIG
        beq dosigrts
        ; put number on the stack as the argument for the signal handler
	; so extend it to 16bit
	clra
	tfr d,x

	lslb		;	2 bytes per entry
        ; load the address of signal handler function
	ldy #U_DATA__U_SIGVEC
	ldu b,y		; now y = udata.u_sigvec[cursig]

        ; udata.u_cursig = 0;
	clr U_DATA__U_CURSIG

        ; restore signal handler to the default.
        ; udata.u_sigvec[cursig] = SIG_DFL;
        ; SIG_DFL = 0
	leay b,y
	clr ,y+
	clr ,y

        ldu #signal_return
        pshs u      ; push return address

        andcc #0xef
	jmp ,x

signal_return:
	puls x
        orcc #0x10
;
;	FIXME: port over the Z80 loop and check for next signal
;
dosigrts:
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

        lds U_DATA__U_ISP

        ; u_data.u_insys = false
        clr U_DATA__U_INSYS
	andcc #0xef
        jmp ,x

;
;	Very simple IRQ handler, we get interrupts at 50Hz and we have to
;	poll ttys from it. The more logic we could move to common here the
;	better.
;
interrupt_handler:
            ; machine state stored by the cpu
	    ; save zero page ??
            ldd _system_tick_counter
	    addd #1
            std _system_tick_counter

	    ; FIXME: add profil support here (need to keep profil ptrs
	    ; unbanked if so ?)

            ; don't allow us to run re-entrant, we've only got one interrupt stack
	    lda U_DATA__U_ININTERRUPT
            bne interrupt_return
            inca
            sta U_DATA__U_ININTERRUPT

            ; switch stacks
            sts istack_switched_sp
	    ; the istack is not banked (very important!)
            lds #istack_top
	    ; FIXME: check store/dec order might need to be -2 here!!!!

	    jsr map_save

	    lda 0		; save address 0 contents for checking

	    ; preserves registers
	    jsr map_kernel
	    ;
	    ; kernel_flag is in the kernel map so we need to map early, we
	    ; need to map anyway for trap_signal
	    ;
	    ldb _kernel_flag
	    pshs b
	    bne in_kernel

            ; we're not in kernel mode, check for signals and fault
	    cmpa #0xA5		;FIXME correct code needed!!
	    beq nofault
	    jsr map_process_always ; map the process
	    lda #0xA5		; put it back
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
            jsr _platform_interrupt

            clr _inint

	    puls b			; Z = in kernel
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

            ; we're not in kernel mode, check for signals
            jsr dispatch_process_signal

interrupt_return:
	    ; restore zero page ??
            andcc #0xef
            rts

;  Enter with X being the signal to send ourself
trap_signal:
	    ldy U_DATA__U_PTAB
	    pshs x,y
            jsr _ssig
	    puls x,y,pc

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
	    pshs d,x,y
	    swi
	    puls d,x,y
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
	jsr map_kernel
        ldx #nmimsg
	jsr outstring
        jsr _trap_monitor


	.area _COMMONMEM
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
;	From the GCC
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
