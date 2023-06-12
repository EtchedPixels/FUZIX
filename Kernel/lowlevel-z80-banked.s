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
	.globl map_kernel
	.globl map_process_always
	.globl map_kernel_di
	.globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl outchar
	.globl _need_resched
	.globl _plt_interrupt
	.globl plt_interrupt_all
	.globl _plt_switchout

        ; exported symbols
	.globl null_handler
	.globl unix_syscall_entry
        .globl _doexec
        .globl trap_illegal
	.globl nmi_handler
	.globl interrupt_handler
	.globl interrupt_legacy
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore
	.globl _in
	.globl _out
	.globl _in16
	.globl _out16
	.globl _sys_cpu
	.globl _sys_cpu_feat
	.globl _sys_stubs
	.globl _set_cpu_type

        ; imported symbols
	.globl _chksigs
	.globl _int_disabled
        .globl _plt_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
	.globl istack_switched_sp
	.globl istack_top
	.globl _ssig
	.globl _udata
	.globl _doexit

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

	ld c,a		; save signal number for called routioe

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
	ld hl,(PROGLOAD+16)
	jp (hl)		; return to user space. This will then return via
			; the return path handler passed in BC

;
;	Syscall signal return path
;
signal_return:
	pop hl		; argument
	di
	;
	;	We must keep IRQ disabled in the kernel mapped
	;	element of this processing, as we don't want to
	;	set INSYS flags here.
	;
	ld (_udata + U_DATA__U_SYSCALL_SP), sp
	ld sp, #kstack_top
	ld a,#1
	ld (_int_disabled),a
	call map_kernel_di
	push af
	call _chksigs
	pop af
	call map_process_always
	ld sp, (_udata + U_DATA__U_SYSCALL_SP)
	jr deliver_signals


;
;	Syscall processing path
;
unix_syscall_entry:
        di
        ; store processor state
        exx
	push bc
        push de
        push hl
        exx
        push bc
        push ix
        push iy
	; We don't save AF / AF' / DE / HL. We do save BC because the 8080 user
	; space will care about that when we unify them.

        ; locate function call arguments on the userspace stack
        ld hl, #16     ; 12 bytes machine state, plus 2 x 2 bytes return address
        add hl, sp
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
	push af
        call _unix_syscall
	pop af

	;
	; WARNING: There are two special cases to beware of here
	; 1. fork() will return twice from _unix_syscall
	; 2. execve() will not return here but will hit _doexec()
	;
	; The fork case returns with a different U_DATA mapped so the
	; U_DATA referencing code is fine, but globals are usually not

        di


	call map_process_always

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
        ; restore machine state
        pop iy
        pop ix
	pop bc
        ; pop af ;; WRS: skip this!
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
        call map_process_always

        pop bc ; return address
	pop af ; bank number
        pop de ; start address

        ld hl, (_udata + U_DATA__U_ISP)
        ld sp, hl      ; Initialize user stack, below main() parameters and the environment

        ; u_data.u_insys = false
        xor a
        ld (_udata + U_DATA__U_INSYS), a

        ex de, hl

	; for the relocation engine - tell it where it is
	; we can generate this from the start address
	ld d,h
	ld e,#0
        ei
        jp (hl)

;
;  Called from process context (hopefully)
;
;  FIXME: hardcoded RST30 won't work on all boxes
;
null_handler:
	; kernel jump to NULL is bad
	ld a, (_udata + U_DATA__U_INSYS)
	or a
	jp nz, trap_illegal
	; user is merely not good
	ld hl, #7
	push hl
	ld ix, (_udata + U_DATA__U_PTAB)
	ld l,P_TAB__P_PID_OFFSET(ix)
	ld h,P_TAB__P_PID_OFFSET+1(ix)
	push hl
	ld hl, #39		; signal (getpid(), SIGBUS)
	call unix_syscall_entry; syscall
	ld hl, #0xFFFF
	push hl
	dec hl			; #0
	push hl
	call unix_syscall_entry; exit



illegalmsg: .ascii "[trap_illegal]"
        .db 13, 10, 0

trap_illegal:
        ld hl, #illegalmsg
        call outstring
        call _plt_monitor

dpsmsg: .ascii "[dispsig]"
        .db 13, 10, 0


nmimsg: .ascii "[NMI]"
        .db 13,10,0

nmi_handler:
	call map_kernel_di
        ld hl, #nmimsg
        call outstring
        jp _plt_monitor

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

	push af
	call _plt_interrupt
	pop af

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

.ifeq PROGBASE
	ld a, (0)
	cp #0xC3
	jp nz, null_pointer_trap
.endif

	; Loop through any pending signals. These could longjmp out
	; of the handler so ensure everything is fixed before this !

	call deliver_signals

	; Then unstack and go.
interrupt_pop:
	xor a
	ld (_int_disabled),a
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
	call map_kernel		; So we call doexit()
	ld sp,#kstack_top	; Need to be off the user stack
				; can't use the interrupt stack as we might
				; IRQ during this
	ld a, #0xC3
	ld (0), a
	ld hl, #9		; SIGKILL
	push hl
	push af
	call _doexit
	pop af
	jp _plt_monitor

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
	ld hl, #intret2
	push hl
	reti			; We have now 'left' the interrupt
				; and the controllers have seen the
				; reti M1 cycle. However we still
				; have DI set

	;
	; We are now on the syscall stack (which is fine, we don't
	; pre-empt mid syscall so therefore it is free.  We will now
	; task switch. The process being pre-empted will disappear into
	; switchout() and whoever is next will come out of the same -
	; hence the need to reti

	;
intret2:di
	call map_kernel_di
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
	push af
	call _chksigs
	pop af

	;
	; Process status is offset 0
	;
	ld hl, (_udata + U_DATA__U_PTAB)
	ld a, #P_RUNNING
	cp (hl)
	jr nz, not_running
	ld (hl), #P_READY
	inc hl
	set PFL_BATCH,(hl)
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
	call map_process_always	; Get our user mapping back

	; We were pre-empted but have now been rescheduled
	; User stack
	ld sp, (_udata + U_DATA__U_SYSCALL_SP)
	ld a, (_udata + U_DATA__U_CURSIG)
	or a
	call nz, deliver_signals_2
	;
	; pop the stack and go
	;
	jr interrupt_pop



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
        call outchar
        ret

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
	pop de
	pop bc
	out (c), b
	push bc
	push de
	jp (hl)

_out16:
	pop hl	; return
	pop iy	; bank
	pop bc	; port
	pop de	; data
	push de
	push bc
	push iy
	out (c),e
	jp (hl)

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
	pop bc
	pop de
	pop hl
	push hl
	push de
	push bc
	di
	ld a,l
	ld (_int_disabled),a
	or a
	ret nz
	ei
	ret

	.area _COMMONMEM

	.globl ___sdcc_enter_ix

___sdcc_enter_ix:
	pop hl		; return address
	push ix		; save frame pointer
	ld ix, #0
	add ix, sp	; set ix to the stack frame
	jp (hl)		; and return

;
;	This must be in common in banked builds
;
	.globl ___sdcc_call_hl
	.globl ___sdcc_call_iy

___sdcc_call_iy:
	.byte 0xFD	; IY override and fall through
___sdcc_call_hl:
	jp (hl)

;
;	These we stick in common as we use them in many places and
;	want to avoid ugly bits. Some are borrowed from sdcc some are
;	done differently for cleanliness. They have been modified to be
;	banking friendly.
;

	.globl _memset

_memset:
	pop af		; ret
	pop iy		; dummy/banking
	pop hl		; dest
	pop de		; fill
	pop bc		; count
	push bc
	push de
	push hl
	push iy
	push af

	ld a, b
	or c
	ret z
	ld (hl), e
	dec a
	ret z		; > 1 ?
	ld d, h

	ld e, l
	inc de
	dec bc
	ldir
	ret

;--------------------------------------------------------------------------
;  divsigned.s
;
;  Copyright (C) 2000-2010, Michael Hope, Philipp Klaus Krause
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

.globl	__divsint
.globl	__divschar

__divsint:
        pop     af
	pop	bc
        pop     hl
        pop     de
        push    de
        push    hl
	push	bc
        push    af

        jp      __div16

__divschar:
        ld      hl, #2+1+2	; banked
        add     hl, sp

        ld      e, (hl)
        dec     hl
        ld      l, (hl)

__div8::
        ld      a, l            ; Sign extend
        rlca
        sbc     a,a
        ld      h, a
__div_signexte::
        ld      a, e            ; Sign extend
        rlca
        sbc     a,a
        ld      d, a
        ; Fall through to __div16

        ;; signed 16-bit division
        ;;
        ;; Entry conditions
        ;;   HL = dividend
        ;;   DE = divisor
        ;;
        ;; Exit conditions
        ;;   HL = quotient
        ;;   DE = remainder
        ;;
        ;; Register used: AF,B,DE,HL
__div16::
        ;; Determine sign of quotient by xor-ing high bytes of dividend
        ;;  and divisor. Quotient is positive if signs are the same, negative
        ;;  if signs are different
        ;; Remainder has same sign as dividend
        ld      a, h            ; Get high byte of dividend
        xor     a, d            ; Xor with high byte of divisor
        rla                     ; Sign of quotient goes into the carry
        ld      a, h            ; Get high byte of dividend
        push    af              ; Save sign of both quotient and reminder

        ; Take absolute value of dividend
        rla
        jr      NC, .chkde      ; Jump if dividend is positive
        sub     a, a            ; Substract dividend from 0
        sub     a, l
        ld      l, a
        sbc     a, a            ; Propagate borrow (A=0xFF if borrow)
        sub     a, h
        ld      h, a

        ; Take absolute value of divisor
.chkde:
        bit     7, d
        jr      Z, .dodiv       ; Jump if divisor is positive
        sub     a, a            ; Subtract divisor from 0
        sub     a, e
        ld      e, a
        sbc     a, a            ; Propagate borrow (A=0xFF if borrow)
        sub     a, d
        ld      d, a

        ; Divide absolute values
.dodiv:
        call    __divu16

.fix_quotient:
        ; Negate quotient if it is negative
        pop     af              ; recover sign of quotient
        ret	NC		; Jump if quotient is positive
        ld      b, a
        sub     a, a            ; Subtract quotient from 0
        sub     a, l
        ld      l, a
        sbc     a, a            ; Propagate borrow (A=0xFF if borrow)
        sub     a, h
        ld      h, a
        ld      a, b
	ret

;--------------------------------------------------------------------------
;  modmixed.s
;
;  Copyright (C) 2010, Philipp Klaus Krause
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

.globl	__moduschar

__moduschar:
	ld      hl,#2+1+2
	ld      d, h
	add     hl,sp

	ld      e,(hl)
	dec     hl
	ld      l,(hl)

	ld      a,l	; Sign extend
	rlca
	sbc     a, a
	ld      h, a

	call	__div16

__get_remainder::
        ; Negate remainder if it is negative and move it into hl
        rla
	ex	de, hl
        ret     NC              ; Return if remainder is positive
        sub     a, a            ; Subtract remainder from 0
        sub     a, l
        ld      l, a
        sbc     a, a             ; Propagate remainder (A=0xFF if borrow)
        sub     a, h
        ld      h, a
        ret



;--------------------------------------------------------------------------
;  divunsigned.s
;
;  Copyright (C) 2000-2012, Michael Hope, Philipp Klaus Krause, Marco Bodrato
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

        ;; Originally from GBDK by Pascal Felber.

.globl	__divuint
.globl	__divuchar

__divuint:
        pop     af
	pop	bc
        pop     hl
        pop     de
        push    de
        push    hl
	push	bc
        push    af

        jr      __divu16

__divuchar:
        ld      hl,#2+1+2
        add     hl,sp

        ld      e,(hl)
        dec     hl
        ld      l,(hl)

        ;; Fall through
__divu8::
        ld      h,#0x00
        ld      d,h
        ; Fall through to __divu16

        ;; unsigned 16-bit division
        ;;
        ;; Entry conditions
        ;;   HL = dividend
        ;;   DE = divisor
        ;;
        ;; Exit conditions
        ;;   HL = quotient
        ;;   DE = remainder
        ;;   carry = 0
        ;;   If divisor is 0, quotient is set to "infinity", i.e HL = 0xFFFF.
        ;;
        ;; Register used: AF,B,DE,HL
__divu16::
        ;; Two algorithms: one assumes divisor <2^7, the second
        ;; assumes divisor >=2^7; choose the applicable one.
        ld      a,e
        and     a,#0x80
        or      a,d
        jr      NZ,.morethan7bits
        ;; Both algorithms "rotate" 24 bits (H,L,A) but roles change.

        ;; unsigned 16/7-bit division
.atmost7bits:
        ld      b,#16           ; bits in dividend and possible quotient
        ;; Carry cleared by AND/OR, this "0" bit will pass trough HL.[*]
        adc     hl,hl
.dvloop7:
        ;; HL holds both dividend and quotient. While we shift a bit from
        ;;  MSB of dividend, we shift next bit of quotient in from carry.
        ;; A holds remainder.
        rla

        ;; If remainder is >= divisor, next bit of quotient is 1.  We try
        ;;  to compute the difference.
        sub     a,e
        jr      NC,.nodrop7     ; Jump if remainder is >= dividend
        add     a,e             ; Otherwise, restore remainder
        ;; The add above sets the carry, because sbc a,e did set it.
.nodrop7:
        ccf                     ; Complement borrow so 1 indicates a
                                ;  successful substraction (this is the
                                ;  next bit of quotient)
        adc     hl,hl
        djnz    .dvloop7
        ;; Carry now contains the same value it contained before
        ;; entering .dvloop7[*]: "0" = valid result.
        ld      e,a             ; DE = remainder, HL = quotient
        ret

.morethan7bits:
        ld      b,#9            ; at most 9 bits in quotient.
        ld      a,l             ; precompute the first 7 shifts, by
        ld      l,h             ;  doing 8
        ld      h,#0
        rr      l               ;  undoing 1
.dvloop:
        ;; Shift next bit of quotient into bit 0 of dividend
        ;; Shift next MSB of dividend into LSB of remainder
        ;; A holds both dividend and quotient. While we shift a bit from
        ;;  MSB of dividend, we shift next bit of quotient in from carry
        ;; HL holds remainder
        adc     hl,hl           ; HL < 2^(7+9), no carry, ever.

        ;; If remainder is >= divisor, next bit of quotient is 1. We try
        ;;  to compute the difference.
        sbc     hl,de
        jr      NC,.nodrop      ; Jump if remainder is >= dividend
        add     hl,de           ; Otherwise, restore remainder
	;; The add above sets the carry, because sbc hl,de did set it.
.nodrop:
        ccf                     ; Complement borrow so 1 indicates a
                                ;  successful substraction (this is the
                                ;  next bit of quotient)
        rla
        djnz    .dvloop
        ;; Take care of the ninth quotient bit! after the loop B=0.
        rl      b               ; BA = quotient
        ;; Carry now contains "0" = valid result.
        ld      d,b
        ld      e,a             ; DE = quotient, HL = remainder
        ex      de,hl           ; HL = quotient, DE = remainder
        ret

;--------------------------------------------------------------------------
;  modunsigned.s
;
;  Copyright (C) 2009-2010, Philipp Klaus Krause
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

.globl	__moduchar
.globl	__moduint

__moduchar:
        ld      hl,#2+1+2
        add     hl,sp

        ld      e,(hl)
        dec     hl
        ld      l,(hl)

        call    __divu8

	ex	de,hl

        ret

__moduint:
        pop     af
	pop	bc
        pop     hl
        pop     de
        push    de
        push    hl
	push	bc
        push    af

        call    __divu16

        ex      de,hl

        ret

;--------------------------------------------------------------------------
;  mulchar.s
;
;  Copyright (C) 2000, Michael Hope
;
;  Modified for banking 2015 Alan Cox
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

.globl	__mulint

__mulint:
        pop     af
	pop	bc
        pop     hl
        pop     de
        push    de
        push    hl
	push	bc
        push    af

        ;; Fall through

	;; Parameters:
	;;	hl, de (left, right irrelevant)
	ld	b,h
	ld	c,l

	;; 16-bit multiplication
	;;
	;; Entry conditions
	;; bc = multiplicand
	;; de = multiplier
	;;
	;; Exit conditions
	;; hl = less significant word of product
	;;
	;; Register used: AF,BC,DE,HL
__mul16::
	xor	a,a
	ld	l,a
	or	a,b
	ld	b,#16

        ;; Optimise for the case when this side has 8 bits of data or
        ;; less.  This is often the case with support address calls.
        jr      NZ,2$
        ld      b,#8
        ld      a,c
1$:
        ;; Taken from z88dk, which originally borrowed from the
        ;; Spectrum rom.
        add     hl,hl
2$:
        rl      c
        rla                     ;DLE 27/11/98
        jr      NC,3$
        add     hl,de
3$:
        djnz    1$
        ret


	.area _CODE1

;--------------------------------------------------------------------------
;  memmove.s
;
;  Copyright (C) 2008-2009, Philipp Klaus Krause, Marco Bodrato
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

	.globl _memmove
	.globl _memcpy
	.globl ___memcpy

; The Z80 has the ldir and lddr instructions, which are perfect for implementing memmove().

_memcpy:
_memmove:
___memcpy:
	pop	af
	pop	iy
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	push	iy
	push	af
	ld	a, c
	or	a, b
	ret	Z
	push	hl
	sbc	hl, de		; or above cleared carry.
	add	hl, de		; same carry as the line before
	jr	C, memmove_up
memmove_down:
	dec	bc
	add	hl, bc
	ex      de, hl
	add	hl, bc
	inc	bc
	lddr
	pop	hl
	ret
memmove_up:
	ex      de, hl
	ldir
	pop	hl
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

	; We'd like this to be discard but for some banked ports that causes
	; link time problems so for now leave it common as it is tiny :
	; FIXME
	.area _COMMONMEM

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
