;
;	Common elements of low level interrupt and other handling. We
; collect this here to minimise the amount of platform specific gloop
; involved in a port
;
; Some features are controlled by Z80_TYPE which should be declared in
; platform/kernel.def as one of the following values:
;     0   CMOS Z80
;     1   NMOS Z80
;     2   Z180
;
;	Based upon code (C) 2013 William R Sowerbutts
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
	.globl map_kernel_di
	.globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl outchar
	.globl _plt_interrupt
	.globl plt_interrupt_all
	.globl _need_resched
	.globl _plt_switchout

        ; exported symbols
	.globl null_handler
	.globl unix_syscall_entry
        .globl _doexec
	.globl nmi_handler
	.globl interrupt_legacy
	.globl interrupt_handler
	.globl synchronous_fault
	.globl ___hard_ei
	.globl ___hard_di
	.globl ___hard_irqrestore
	.globl _out
	.globl _in
	.globl _out16
	.globl _in16
	.globl _sys_cpu
	.globl _sys_cpu_feat
	.globl _sys_stubs
	.globl _set_cpu_type

	.globl mmu_irq_ret

        ; imported symbols
	.globl _chksigs
	.globl _int_disabled
        .globl _plt_monitor
	.globl _doexit
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
	.globl istack_switched_sp
	.globl istack_top
	.globl _ssig
	.globl _udata

        .include "platform/kernel.def"
        .include "kernel-z80.def"

; these make the code below more readable. sdas allows us only to 
; test if an expression is zero or non-zero.
CPU_Z180	    .equ    Z80_TYPE-2

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
	or a
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

	ld bc, #signal_return
	push bc		; bc is passed in as the return vector

	ld c,a		; Save signal number in C as well for 8080 style

	; Indicate processed
	xor a
	ld (_udata + U_DATA__U_CURSIG), a
	; and we will handle the signal with interrupts on so clear the
	; flag
	ld (_int_disabled),a

	; Semantics for now: signal delivery clears handler
	ld (hl), a
	dec hl
	ld (hl), a


	ei
	.ifne Z80_MMU_HOOKS
	call mmu_user		; must preserve HL
	.endif
	ld hl,(PROGLOAD+16); return to user space. This will then return via
			; the return path handler stacked above via BC
	jp (hl)
;
;	Syscall signal return path
;
signal_return:
	pop hl		; argument
	di
	.ifne Z80_MMU_HOOKS
	call mmu_kernel
	.endif
	;
	;	We must keep IRQ disabled in the kernel mapped
	;	element of this processing, as we don't want to
	;	set INSYS flags here.
	;
	ld (_udata + U_DATA__U_SYSCALL_SP), sp
	ld sp, #kstack_top
	;
	;	Ensure chksigs and friends see the right status
	;
	ld a,#1
	ld (_int_disabled),a
	call map_kernel_di
	call _chksigs
	call map_process_always_di
	ld sp, (_udata + U_DATA__U_SYSCALL_SP)
	jr deliver_signals


;
;	Syscall processing path
;
;	This is the first part of the big API change. This is in effect
;	a temporary ABI so don't build stuff to it!
;
unix_syscall_entry:
	; We know the previous state was EI and that we won't do anything
	; clever until we EI again, so we can avoid the helpers on the fast
	; path.
        di
        ; store processor state. We destroy AF, AF', DE, HL
        exx
        push bc
        push de
        push hl
        exx
	push bc
        push ix
        push iy

        ; locate function call arguments on the userspace stack
        ld hl, #16     ; 12 bytes machine state, plus 2 bytes return address x 2
        add hl, sp

	.ifne Z80_MMU_HOOKS
	call mmu_kernel			; must preserve A,HL
	.endif
        ; save system call number
        ld (_udata + U_DATA__U_CALLNO), a
        ; advance to syscall arguments
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
	call map_kernel_di

        ; re-enable interrupts
        ei

        ; now pass control to C
        call _unix_syscall

	;
	; WARNING: There are two special cases to beware of here
	; 1. fork() will return twice from _unix_syscall
	; 2. execve() will not return here but will hit _doexec()
	;
	; The fork case returns with a different U_DATA mapped so the
	; U_DATA referencing code is fine, but globals are usually not

        di	; Again we know we won't mess up calling di/ei directly


	; FIXME: another spot we di but have the flag wrong and call stuff
	; although we probably just need a rule that _di versions can't
	; rely on it!
	call map_process_always_di

	xor a
	ld (_udata + U_DATA__U_INSYS), a

	; Back to the user stack
	ld sp, (_udata + U_DATA__U_SYSCALL_SP)

	ld hl, (_udata + U_DATA__U_ERROR)
	ld de, (_udata + U_DATA__U_RETVAL)

	ld a, (_udata + U_DATA__U_CURSIG)
	or a

	; Fast path the normal case
	jr nz, via_signal

	; Restore stacks and go
	;
	; Should we change the ABI and just return in DE/HL ?
	;
unix_return:
	ld a, h
	or l
	jr z, not_error
	scf		; carry flag on return state for errors
	jr unix_pop

not_error:
	ex de, hl	; return the retval instead
	;
	; Undo the stacking and go back to user space
	;
unix_pop:
	.ifne Z80_MMU_HOOKS
	call mmu_user		; must preserve HL
	.endif
        ; restore machine state
        pop iy
        pop ix
	pop bc
        exx
        pop hl
        pop de
        pop bc
        exx
        ei
        ret ; must immediately follow EI


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
        di
        call map_process_always_di

        pop bc ; return address
        pop de ; start address

        ld hl, (_udata + U_DATA__U_ISP)
        ld sp, hl      ; Initialize user stack, below main() parameters and the environment

        ; u_data.u_insys = false
        xor a
        ld (_udata + U_DATA__U_INSYS), a

        ex de, hl

	.ifne Z80_MMU_HOOKS
	call mmu_user		; must preserve HL
	.endif
	; for the relocation engine - tell it where it is
	; we always start in the low 256 bytes of our binary so we can
	; just generate the relocation base accordingly
	ld d,h
	ld e,#0
        ei
        jp (hl)

;
;	We took a NULL pointer - either a branch to NULL or on Z180
;	an illegal. For now use a blunt instrument. We could in theory
;	do a nicer signal throw etc.
;
null_handler:
	di
	; kernel jump to NULL is bad
	ld a, (_udata + U_DATA__U_INSYS)
	or a
	jp nz, trap_illegal
	ld a, (_udata + U_DATA__U_ININTERRUPT)
	or a
	jp nz, trap_illegal
	; user is merely not good - handle it synchronously
	ld hl,#9		; SIGKILL
synchronous_fault:
	ld sp,#kstack_top
	call map_kernel_di
	push hl
	call _doexit
trap_illegal:
        ld hl, #illegalmsg
traphl:
        call outstring
        call _plt_monitor

illegalmsg: .ascii "[illegal]"
            .db 13, 10, 0

nmimsg: .ascii "[NMI]"
        .db 13,10,0

nmi_handler:
	.ifne Z80_MMU_HOOKS
	call mmu_kernel
	.endif
	call map_kernel_di
        ld hl, #nmimsg
	jr traphl

;
;	Interrupt handler. Not quite the same as syscalls, we need to
;	stack everything and we must get off the IRQ stack and then
;	process need_resched and signals
;
interrupt_legacy:
.ifeq CPU_Z180
	; Make sure we mark the Z80 legacy interrupt as vector FF
	ex af,af'
	push af
	xor a
	ld (hw_irqvector),a
	jr intvec
.endif
interrupt_handler:
        ; store machine state
        ex af,af'
        push af
intvec:
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
	.ifne Z80_MMU_HOOKS
	ld hl, #mmu_irq_ret
	jp mmu_kernel_irq
	.endif
mmu_irq_ret:

	; Some platforms (MSX for example) have devices we *must*
	; service irrespective of kernel state in order to shut them
	; up. This code must be in common and use small amounts of stack
	call plt_interrupt_all
	; FIXME: add profil support here (need to keep profil ptrs
	; unbanked if so ?)
.ifeq CPU_Z180
        ; On Z180 we have more than one IRQ, so we need to track of which one
        ; we arrived through. The IRQ handler sets irqvector_hw when each
        ; interrupt arrives. If we are not already handling an interrupt then
        ; we copy this into _irqvector which is the value the kernel code
        ; examines (and will not change even if reentrant interrupts arrive).
        ; Generally the only place that irqvector_hw should be used is in
        ; the plt_interrupt_all routine.
        .globl hw_irqvector
        .globl _irqvector
        ld a, (hw_irqvector)
        ld (_irqvector), a
.endif

	; Get onto the IRQ stack
	ld (istack_switched_sp), sp
	ld sp, #istack_top

	call map_save_kernel

	ld a,#1
	; So we know that this task should resume with IRQs off
	ld (_udata + U_DATA__U_ININTERRUPT), a
	; Load the interrupt flag properly. It got an implicit di from
	; the IRQ being taken
	ld (_int_disabled),a

	call _plt_interrupt

	ld a, (_need_resched)
	or a
	jr nz, preemption

	; Back to the old memory map
	call map_restore

	;
	; Back on user stack
	;
	ld sp, (istack_switched_sp)

intout:
	xor a
	ld (_udata + U_DATA__U_ININTERRUPT), a
	;
	;	Z180 internal interrupts do not reti
	;
.ifeq CPU_Z180
	ld a,(_irqvector)
	or a
	jr nz, intret
.endif
	ld hl, #intret
	push hl
	reti			; We have now 'left' the interrupt
				; and the controllers have seen the
				; reti M1 cycle. However we still
				; have DI set
intret:
	di
	ld a, (_udata + U_DATA__U_INSYS)
	or a
	jr nz, interrupt_pop

	ld a,(0)
	cp #0xC3
	jp nz, null_pointer_trap
	; Loop through any pending signals. These could longjmp out
	; of the handler so ensure everything is fixed before this !

	call deliver_signals

	; Then unstack and go.
interrupt_pop:
	xor a
	ld (_int_disabled),a
	.ifne Z80_MMU_HOOKS
	call mmu_restore_irq
	.endif
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
        ei			; Must be instruction before ret
	ret			; runs in the ei interrupt shadow

;
;	At the point we fire we are back on the user stack and logically
;	speaking have just finished the interrupt. If the low byte was
;	corrupt we assume the worst and just blow the process away
;
null_pointer_trap:
	ld sp,#kstack_top	; Need to be off the user stack
				; can't use the interrupt stack as we might
				; IRQ during this
	call map_kernel_di	; to deliver the kill
	ld a, #0xC3		; Repair
	ld (0), a
	ld hl, #9		; SIGKILL (take no prisoners here)
trap_signal:
	push hl
	ld hl,(_udata + U_DATA__U_PTAB)
	ld de, #P_TAB__P_PID_OFFSET
	add hl,de
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	push de
        call _ssig
	; Now fall into pre-emption from which we will not return
;
;	Pre-emption. We need to get off the interrupt stack, switch task
;	and clean up the IRQ state carefully
;

	.globl preemption

preemption:
	xor a
	ld (_need_resched), a	; Task done

	; Back to the old memory map
	call map_restore

	ld hl, (istack_switched_sp)
	ld (_udata + U_DATA__U_SYSCALL_SP), hl

	ld sp, #kstack_top	; We don't pre-empt in a syscall
				; so this is fine
.ifeq CPU_Z180
	ld a,(_irqvector)
	or a
	jr nz, intret2
.endif
	ld hl, #intret2
	push hl
	reti			; We have now 'left' the interrupt
				; and the controllers have seen the
				; reti M1 cycle. However we still
				; have DI set
	di			; see undocumented Z80 notes on RETI
	;
	; We are now on the syscall stack (which is fine, we don't
	; pre-empt mid syscall so therefore it is free.  We will now
	; task switch. The process being pre-empted will disappear into
	; switchout() and whoever is next will come out of the same -
	; hence the need to reti

	;
intret2:call map_kernel_di
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
	ld a,#P_RUNNING
	cp (hl)
	jr nz, not_running
	ld (hl), #P_READY
	inc hl
	set #PFL_BATCH,(hl)
not_running:
	call _plt_switchout
	;
	; We are no longer in an interrupt or a syscall
	;
	xor a
	ld (_udata + U_DATA__U_ININTERRUPT), a
	ld (_udata + U_DATA__U_INSYS), a
	;
	; We have been rescheduled, remap ourself and go back to user
	; space via signal handling
	;
	call map_process_always_di ; Get our user mapping back


	; We were pre-empted but have now been rescheduled
	; User stack
	ld sp, (_udata + U_DATA__U_SYSCALL_SP)
	ld a, (_udata + U_DATA__U_CURSIG)
	or a
	call nz, deliver_signals_2
	;
	; pop the stack and go
	;
	.ifne Z80_MMU_HOOKS
	call mmu_user
	.endif
	jp interrupt_pop

;
;	Debugging helpers
;

	.area _COMMONMEM

; outstring: Print the string at (HL) until 0 byte is found
; destroys: AF HL
outstring:
        ld a, (hl)     ; load next character
        and a          ; test if zero
        ret z          ; return when we find a 0 byte
        call outchar
        inc hl         ; next char please
        jr outstring

; print the string at (HL) in hex (continues until 0 byte seen)
outstringhex:
        ld a, (hl)     ; load next character
        and a          ; test if zero
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
        and #0x0f ; mask off low four bits
        cp #10
        jr c, numeral ; less than 10?
        add a, #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:add a, #0x30 ; start at '0' (0x30='0')
        jp outchar

;
;	I/O helpers for cases we don't use __sfr
;
_out:
	pop hl
	pop bc
	out (c), b
	push bc
	jp (hl)

_out16:
	pop hl
	pop bc
	pop de
	push de
	push bc
	out (c),e
	jp (hl)

;
;	Use z88dk_fastcall for in.
;
_in16:
	ld b,h
_in:
	ld c,l
	in l, (c)
	ret

;
;	Deal with all the NMOS Z80 bugs and the buggy emulators by
;	simply tracing our own interrupt status. It's cheaper this way
;	but does mean any code that is using di and friends directly needs
;	to be a lot more careful. We can also make irqflags_t 8bit and
;	fastcall the irqrestore later on FIXME
;
___hard_ei:
	xor a
	ld (_int_disabled),a
	ei
	ret

___hard_di:
	ld hl,#_int_disabled
	di
	ld a,(hl)
	ld (hl),#1
	ld l,a
	ret

___hard_irqrestore:
	pop de
	pop hl
	push hl
	push de
	di
	ld a,l
	ld (_int_disabled),a
	or a
	ret nz
	ei
	ret

	.area _CONST

_sys_stubs:
	jp unix_syscall_entry
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

	.area _DATA

_sys_cpu:
	.db 0
_sys_cpu_feat:
	.db 0

	.area _DISCARD

_set_cpu_type:
	ld h,#2		; Assume Z80
	xor a
	dec a
	daa
	jr c,is_z80
	ld h,#6		; Nope Z180
is_z80:
	ld l,#1		; 8080 family
	ld (_sys_cpu),hl	; Write cpu and cpu feat
	ret
