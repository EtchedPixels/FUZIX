;
;	Rabbit 2/3000 low level interface code
;
;	Heavily based on the Z80/Z180 code
;
;	Try and keep any CPU specifics elsewhere.
;

		.module lowlevel

		; debugging aids
		.globl outcharhex
		.globl outbc, outde, outhl
		.globl outnewline
		.globl outstring
		.globl outstringhex
		.globl outnibble

		; platform provided functions
		.globl map_kernel
		.globl map_process_always
	        .globl map_save_kernel
       		.globl map_restore
		.globl outchar
		.globl _plt_interrupt
		.globl plt_interrupt_all
	        .globl _plt_monitor
		.globl _plt_switchout

        	; exported symbols
		.globl null_handler
		.globl unix_syscall_entry
	        .globl _doexec
	        .globl trap_illegal
		.globl interrupt_handler
		.globl ___hard_ei
		.globl ___hard_di
		.globl ___hard_irqrestore

	        ; imported symbols
	        .globl _unix_syscall
	        .globl outstring
	        .globl kstack_top
		.globl istack_switched_sp
		.globl istack_top
		.globl _ssig
		.globl _doexit
		.globl _need_resched
		.globl _chksigs
		.globl _udata

	        .include "platform/kernel.def"
	        .include "kernel-rabbit.def"

		.area _COMMONMEM

;
;	Called on the user stack in order to process signals that
;	are pending. A user process can longjmp out of this loop so
;	care is needed. Call with interrupts disabled and user mapped.
;
;	Returns with interrupts disabled and user mapped, but may
;	enable interrupts and change mappings.
;
deliver_signals:
		; Pending signal
		ld a, (_udata + U_DATA__U_CURSIG)
		or a,a
		ret z

deliver_signals_2:
		ld l, a
		ld h, #0
		push hl		; signal number as C argument to the handler

		; Handler to use
		add hl, hl
		ld de, #_udata + U_DATA__U_SIGVEC
		add hl, de
		ld e, (hl)
		inc hl
		ld d,(hl)

		; Indicate processed
		xor a,a
		ld (_udata + U_DATA__U_CURSIG), a

		; Semantics for now: signal delivery clears handler
		ld (hl), a
		dec hl
		ld (hl), a

		ld bc, #signal_return
		push bc		; bc is passed in as the return vector

		ex de, hl
		ipset0
;;FIX	call mmu_user		; must preserve HL
		jp (hl)		; return to user space. This will then return via
				; the return path handler passed in BC

;
;	Syscall signal return path
;
signal_return:
		pop hl		; argument
		ipset1
;;FIX		call mmu_kernel
		;
		;	We must keep IRQ disabled in the kernel mapped
		;	element of this processing, as we don't want to
		;	set INSYS flags here.
		;
		ld (_udata + U_DATA__U_SYSCALL_SP), sp
		ld sp, #kstack_top
		call map_kernel
		call _chksigs
		call map_process_always
		ld sp, (_udata + U_DATA__U_SYSCALL_SP)
		jr deliver_signals

;
;	Syscall processing path
;
;	FIXME: rework to undo the Z80 mistakes. Pass arguments in registers
;	don't screw up stack. Check how our trap arrives and push pop ip etc
;	as needed for different stack layout.
;
unix_syscall_entry:
        	ipset1
        	; store processor state
	        ex af, af'
        	push af
        	ex af, af'
        	exx
	        push bc
	        push de	
	        push hl
	        exx
	        push ix
	        push iy
		; We don't save AF BC DE HL
	        ; locate function call arguments on the userspace stack
	        ld hl, #20      ; 16 bytes machine state, plus 2 bytes return address
				; plus 2 bytes return frame for the syscall stub
	        add hl, sp

;;		call mmu_kernel		; must preserve HL
	        ; save system call number
	        ld a, (hl)
	        ld (_udata + U_DATA__U_CALLNO), a
	        ; advance to syscall arguments
	        inc hl
	        inc hl
	        ; copy arguments to common memory
       		ld de, #_udata + U_DATA__U_ARGN
	        ldi
		ldi
		ldi
		ldi
		ldi
		ldi
		ldi
		ldi

		ld a, #1
		ld (_udata + U_DATA__U_INSYS), a

        	; save process stack pointer
        	ld (_udata + U_DATA__U_SYSCALL_SP), sp
        	; switch to kernel stack
        	ld sp, #kstack_top

	        ; map in kernel keeping common
		call map_kernel

	        ; re-enable interrupts
        	ipres

	        ; now pass control to C
	        call _unix_syscall

		;
		; WARNING: There are two special cases to beware of here
		; 1. fork() will return twice from _unix_syscall
		; 2. execve() will not return here but will hit _doexec()
		;
		; The fork case returns with a different U_DATA mapped so the
		; U_DATA referencing code is fine, but globals are usually not

        	ipset1


		call map_process_always

		xor a,a
		ld (_udata + U_DATA__U_INSYS), a

		; Back to the user stack
		ld sp, (_udata + U_DATA__U_SYSCALL_SP)

		ld hl, (_udata + U_DATA__U_ERROR)
		ld de, (_udata + U_DATA__U_RETVAL)

		ld a, (_udata + U_DATA__U_CURSIG)
		or a,a

		; Fast path the normal case
		jr nz, via_signal

		; Restore stacks and go
		;
		; Should we change the ABI and just return in DE/HL ?
		;
unix_return:
		ld a, h
		or a,l
		jr z, not_error
		scf		; carry flag on return state for errors
		jr unix_pop

not_error:
		ex de, hl	; return the retval instead
		;
		; Undo the stacking and go back to user space
		;
unix_pop:
;;		call mmu_user		; must preserve HL
	        ; restore machine state
	        pop iy	
	        pop ix	
	        exx
	        pop hl
	        pop de
	        pop bc
	        exx
	        ex af, af'
	        pop af
	        ex af, af'
	        reti

via_signal:
		; Get off the kernel syscall stack before we start signal
		; handling. Our signal handlers may themselves elect to make system
		; calls. This means we must also save the error/return code
		ld hl, (_udata + U_DATA__U_ERROR)
		push hl
		ld hl, (_udata + U_DATA__U_RETVAL)
		push hl

		; Signal processing. This may longjmp back into userland
		call deliver_signals_2

		; If not then we recover the syscall return values and
		; exit via the syscall return path
		pop de			; retval
		pop hl			; errno
		jr unix_return


;
;	Final component of execve()
;
_doexec:
	        ipset1
	        call map_process_always

	        pop bc ; return address
	        pop de ; start address

	        ld hl, (_udata + U_DATA__U_ISP)
	        ld sp, hl      ; Initialize user stack, below main() parameters and the environment

	        ; u_data.u_insys = false
	        xor a,a
	        ld (_udata + U_DATA__U_INSYS), a

	        ex de, hl
		ld d,h
		ld e,#0
		; for the relocation engine - tell it where it is
;;	call mmu_user		; must preserve HL
	        ipres
	        jp (hl)

;
;  Called from process context (hopefully)
;
null_handler:
		; kernel jump to NULL is bad
		ld a, (_udata + U_DATA__U_INSYS)
		or a,a
		jp nz, trap_illegal
		ld a, (_udata + U_DATA__U_ININTERRUPT)
		jp nz, trap_illegal
		; user is merely not good
		ld hl, #7
		push hl
		ld ix, (_udata + U_DATA__U_PTAB)
		ld l,P_TAB__P_PID_OFFSET(ix)
		ld h,P_TAB__P_PID_OFFSET+1(ix)
		push hl
		ld hl, #39		; signal (getpid(), SIGBUS)
		push hl
		call unix_syscall_entry		; syscall
		ld hl, #0xFFFF
		call _doexit		; never returns


illegalmsg: 	.ascii "[illegal]"
            	.db 13, 10, 0

trap_illegal:
	        ld hl, #illegalmsg
traphl:
	        call outstring
	        call _plt_monitor

;
;	Interrupt handler. Not quite the same as syscalls, we need to
;	stack everything and we must get off the IRQ stack and then
;	process need_resched and signals
;
interrupt_handler:
	        ; store machine state
		; FIXME: who saves XPC - map_save ??
	        ex af,af'
	        push af
	        ex af,af'
	        exx
	        push bc
	        push de
	        push hl
	        exx
	        push af
	        push bc
	        push de
	        push hl
	        push ix
	        push iy
		;
		; This is a bit exciting - if our MMU enforces r/o then the entire
		; stack state might be bogus!
		;
;;		ld hl, #mmu_irq_ret
;;		jp mmu_kernel_irq
;;mmu_irq_ret:
		; FIXME: add profil support here (need to keep profil ptrs
		; unbanked if so ?)

		; Get onto the IRQ stack
		ld (istack_switched_sp), sp
		ld sp, #istack_top

		ld a, (0)

		call map_save_kernel

		cp a,#0xC3
		jr z, no_null_ptr
		call null_pointer_trap
no_null_ptr:
		; So we know that this task should resume with IRQs off
		ld (_udata + U_DATA__U_ININTERRUPT), a

		call _plt_interrupt

		ld a, (_need_resched)
		or a,a
		jr nz, preemption

		; Back to the old memory map
		call map_restore

		;
		; Back on user stack
		;
		ld sp, (istack_switched_sp)

intout:
		xor a,a
		ld (_udata + U_DATA__U_ININTERRUPT), a

intret:
		ipset1
		ld a, (_udata + U_DATA__U_INSYS)
		or a,a
		jr nz, interrupt_pop

		; Loop through any pending signals. These could longjmp out
		; of the handler so ensure everything is fixed before this !

		call deliver_signals
;;		call mmu_restore_irq

	; Then unstack and go.
interrupt_pop:
	        pop iy
	        pop ix
	        pop hl
	        pop de
	        pop bc
	        pop af
	        exx
	        pop hl
	        pop de
	        pop bc
	        exx
	        ex af, af'
	        pop af
	        ex af, af'
		reti			; runs in the ei interrupt shadow

;
;	Called with the kernel mapped, mid interrupt and on the IRQ stack
;
null_pointer_trap:
		ld a, #0xC3
		ld (0), a
		ld hl, #11		; SIGSEGV
trap_signal:
		push hl
		ld hl, (_udata + U_DATA__U_PTAB);
		push hl
	        call _ssig
        	pop hl
	        pop hl
		ret


;
;	Pre-emption. We need to get off the interrupt stack, switch task
;	and clean up the IRQ state carefully
;

	.globl preemption

preemption:
		xor a,a
		ld (_need_resched), a	; Task done

		; Back to the old memory map
		call map_restore

		ld hl, (istack_switched_sp)
		ld (_udata + U_DATA__U_SYSCALL_SP), hl

		ld sp, #kstack_top	; We don't pre-empt in a syscall
				; so this is fine

		ipset1
		;
		; We are now on the syscall stack (which is fine, we don't
		; pre-empt mid syscall so therefore it is free.  We will now
		; task switch. The process being pre-empted will disappear into
		; switchout() and whoever is next will come out of the same -
		; hence the need to reti
		;
		call map_kernel
		;
		; Semantically we are doing a null syscall for pre-empt. We need
		; to record ourselves as in a syscall so we can't be recursively
		; pre-empted when switchout re-enables interrupts.
		;
		ld a, #1
		ld (_udata + U_DATA__U_INSYS), a
		;
		; Check for signals
		;
		call _chksigs
		;
		; Process status is offset 0
		;
		ld hl, (_udata + U_DATA__U_PTAB)
		ld a,(hl)
		cp a,#P_RUNNING
		jr nz, not_running
		ld (hl), #P_READY
		inc hl
		set PFL_BATCH,(hl)
not_running:
		call _plt_switchout
		;
		; We are no longer in an interrupt or a syscall
		;
		xor a,a
		ld (_udata + U_DATA__U_ININTERRUPT), a
		ld (_udata + U_DATA__U_INSYS), a
		;
		; We have been rescheduled, remap ourself and go back to user
		; space via signal handling
		;
		call map_process_always	; Get our user mapping back
		; We were pre-empted but have now been rescheduled
		; User stack
		ld sp, (_udata + U_DATA__U_SYSCALL_SP)
		ld a, (_udata + U_DATA__U_CURSIG)
		or a,a
		jr z, nosigs
		call deliver_signals_2
nosigs:
		;
		; pop the stack and go
		;
;;		call mmu_user
		jr interrupt_pop

;
;	Debugging helpers
;

		.area _COMMONMEM

; outstring: Print the string at (HL) until 0 byte is found
; destroys: AF HL
outstring:
	        ld a, (hl)     ; load next character
	        and a,a        ; test if zero
	        ret z          ; return when we find a 0 byte
	        call outchar
	        inc hl         ; next char please
	        jr outstring

; print the string at (HL) in hex (continues until 0 byte seen)
outstringhex:
	        ld a, (hl)     ; load next character
	        and a,a        ; test if zero
	        ret z          ; return when we find a 0 byte
	        call outcharhex
	        ld a, #0x20 ; space
	        call outchar
	        inc hl         ; next char please
	        jr outstringhex

; output a newline
outnewline:
	        ld a, #0x0d  ; output newline
	       	call outchar
	        ld a, #0x0a
	        jp outchar

outhl:  ; prints HL in hex.
		push af
	        ld a, h
	        call outcharhex
	        ld a, l
	        call outcharhex
		pop af
	        ret

outbc:  ; prints BC in hex.
		push af
	        ld a, b
	        call outcharhex
	        ld a, c
	        call outcharhex
		pop af
	        ret

outde:  ; prints DE in hex.
		push af
	        ld a, d
	        call outcharhex
	        ld a, e
	        call outcharhex
		pop af
	        ret

; print the byte in A as a two-character hex value
outcharhex:
	        push bc
		push af
	        ld c, a  ; copy value
	        ; print the top nibble
	        rra
	        rra
	        rra
	        rra
	        call outnibble
	        ; print the bottom nibble
	        ld a, c
	        call outnibble
		pop af
	        pop bc
	        ret

; print the nibble in the low four bits of A
outnibble:
	        and a,#0x0f ; mask off low four bits
	        cp a,#10
	        jr c, numeral ; less than 10?
	        add a, #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:	add a, #0x30 ; start at '0' (0x30='0')
        	jp outchar

;
; IRQ helpers, in common as they may get used by common C
; code (and are tiny)
;
; We use the multiple IRQ levels as our low level code needs to do things
; like soft buffer uarts
;
_ei:
___hard_ei:
		ipset0
		ret
;
; This needs review. If we are careful about our root bank of 4K we may be
; able to stuff all our vectors and handling for critical IRQ code there and
; *never* disable interrupts except for the tiny bits of code working the
; serial queues !
;
___hard_di:	push ip
		pop hl
		ipset3		; we block everything for hard_di
		ret

_irqrestore:
___hard_irqrestore:
		pop hl
		pop ip
		jp (hl)

_di:		push ip
		pop hl
		ipset1		; check timer priority we are using and what
				; we can fit into the low space
		ret

;
; Read from I/O space. Allow for the R2K erratum
;
		.globl _in
		.globl _out

_in:
		ioi
		ld a,(hl)
		nop
		ret

_out:
		ld hl,8(sp)
		ex de,hl
		ld hl,4(sp)
		ioi
		ld (hl),e
		nop
		ret
		ld hl,4(sp)

;
;	CPU info
;
	.area _DATA

	.globl	_sys_cpu
	.globl	_sys_cpu_feat
	.globl	_set_cpu_type
	.globl	_sys_stubs

_sys_cpu:
	.db	0
_sys_cpu_feat:
	.db	0

	.area _DISCARD

_set_cpu_type:
	; TODO
	ret

		.area _CONST
_sys_stubs:
	jp	unix_syscall_entry
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

