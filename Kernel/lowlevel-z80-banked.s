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
        .globl map_save
        .globl map_restore
	.globl outchar
	.globl _kernel_flag
	.globl _inint
	.globl _platform_interrupt
	.globl platform_interrupt_all

        ; exported symbols
        .globl unix_syscall_entry
	.globl _chksigs
	.globl null_handler
	.globl unix_syscall_entry
        .globl _doexec
        .globl trap_illegal
	.globl nmi_handler
	.globl interrupt_handler
	.globl _di
	.globl _irqrestore
	.globl _in
	.globl _out

        ; imported symbols
        .globl _trap_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
	.globl istack_switched_sp
	.globl istack_top
	.globl _ssig

        .include "platform/kernel.def"
        .include "kernel.def"

; these make the code below more readable. sdas allows us only to 
; test if an expression is zero or non-zero.
CPU_CMOS_Z80	    .equ    Z80_TYPE-0
CPU_NMOS_Z80	    .equ    Z80_TYPE-1
CPU_Z180	    .equ    Z80_TYPE-2

        .area _COMMONMEM

; entry point for UZI system calls
unix_syscall_entry:
        di
        ; store processor state
        ex af, af'
        push af
        ex af, af'
        exx
        push bc
        push de
        push hl
        exx
        ; push af ;; WRS: also skip this
        push bc
        push de
        ; push hl ;; WRS: we could skip this since we always set HL on return
        push ix
        push iy

        ; locate function call arguments on the userspace stack
        ld hl, #18     ; 16 bytes machine state, plus 2 bytes return address
        add hl, sp
        ; save system call number
        ld a, (hl)
        ld (U_DATA__U_CALLNO), a
        ; advance to syscall arguments
        inc hl
        inc hl
        ; copy arguments to common memory
        ld bc, #8      ; four 16-bit values
        ld de, #U_DATA__U_ARGN
        ldir           ; copy

        ; save process stack pointer
        ld (U_DATA__U_SYSCALL_SP), sp
        ; switch to kernel stack
        ld sp, #kstack_top

        ; map in kernel keeping common
	call map_kernel

	; make sure the interrupt logic knows we are in kernel mode (flag lives in kernel bank)
	ld a, #1
	ld (_kernel_flag), a

        ; re-enable interrupts
        ei

        ; now pass control to C
	push af
        call _unix_syscall
	pop af

        di
	; let the interrupt logic know we are not in kernel mode any more
	; kernel_flag is not in common so write it before we map it away
	xor a
	ld (_kernel_flag), a
	;
	; We restart from here on a unix syscall signal return path
	;
unix_sig_exit:
        ; map process memory back in based on common (common may have
        ; changed on a task switch)
        call map_process_always

        ; switch back to user stack
        ld sp, (U_DATA__U_SYSCALL_SP)

        ; check for signals, call the handlers
        call dispatch_process_signal

        ; check if error condition to be signalled on return
        ld hl, (U_DATA__U_ERROR)
        ld a, h
        or l    ; set NZ flag if we are to return error
        jr z, not_error

        scf    ; set carry flag
        ; note error code remains in HL
        jr unix_return

not_error:
        ; no error to signal! return syscall return value instead of error code
        ld hl, (U_DATA__U_RETVAL)
        ; fall through to return code

unix_return:
        ; restore machine state
        pop iy
        pop ix
        ; pop hl ;; WRS: skip this!
        pop de
        pop bc
        ; pop af ;; WRS: skip this!
        exx
        pop hl
        pop de
        pop bc
        exx
        ex af, af'
        pop af
        ex af, af'
        ei
        ret ; must immediately follow EI

dispatch_process_signal:
        ; check if any signal outstanding
        ld a, (U_DATA__U_CURSIG)
        or a
        ret z

        ; put system call number on the stack as the argument for the signal handler
        ld l, a
        ld h, #0
        push hl

        ; load the address of signal handler function
        add hl, hl
        ld de, #U_DATA__U_SIGVEC
        add hl, de
        ld e, (hl)
        inc hl
        ld d, (hl)      ; now DE = udata.u_sigvec[cursig]

        ; udata.u_cursig = 0;
        xor a
        ld (U_DATA__U_CURSIG), a

        ; restore signal handler to the default.
        ; udata.u_sigvec[cursig] = SIG_DFL;
        ; SIG_DFL = 0, A is still 0 from above, HL points at second byte of the signal vector.
        ld (hl), a
        dec hl
        ld (hl), a

        ld hl, #signal_return
        push hl      ; push return address

        ex de,hl
        ei
        jp (hl)        ; call signal handler in userspace, with interrupts enabled

signal_return:
        pop hl  ; remove arg from stack
	pop hl  ; we won't be using the return address either
        di	; So we don't screw up in mapping and stack games
        ; save process stack pointer
        ld (U_DATA__U_SYSCALL_SP), sp
        ; switch to kernel stack
        ld sp, #kstack_top
	call map_kernel
	push af
	call _chksigs
	pop af
	; Loop back around, switch stacks, check if there is a signal
	; and if so process it.
	;
	; If we do restartable signals we can just check the restartable
	; info and jmp back further up the syscall path *providing* that
	; on signal exit paths we write back any needed parameters with
	; their new info
	jr unix_sig_exit

_doexec:
        di
	; this is a funny extra path out of syscall so we must also cover
	; the exit from kernel here
	xor a
	ld (_kernel_flag), a
        ; map task into address space (kernel_flag is no longer mapped, don't
	; re-order this)
        call map_process_always

	pop af ; pop far data
        pop bc ; return address
        pop de ; start address
        ;; push de ; restore stack ... but we're about to discard SP anyway!
        ;; push bc 

        ld hl, (U_DATA__U_ISP)
        ld sp, hl      ; Initialize user stack, below main() parameters and the environment

        ; u_data.u_insys = false
        xor a
        ld (U_DATA__U_INSYS), a

        ex de, hl

	; for the relocation engine - tell it where it is
	ld iy, #PROGLOAD
        ei
        jp (hl)

;
;	Very simple IRQ handler, we get interrupts at 50Hz and we have to
;	poll ttys from it. The more logic we could move to common here the
;	better.
;
interrupt_handler:
            ; store machine state
            ; we arrive here via the trampoline at 0038h with interrupts disabled
            ; save CPU registers (restored in _IRET)
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

	    ; Some platforms (MSX for example) have devices we *must*
	    ; service irrespective of kernel state in order to shut them
	    ; up. This code must be in common and use small amounts of stack
	    call platform_interrupt_all
	    ; FIXME: add profil support here (need to keep profil ptrs
	    ; unbanked if so ?)

            ; don't allow us to run re-entrant, we've only got one interrupt stack
            ld a, (U_DATA__U_ININTERRUPT)
            or a
            jp nz, interrupt_return
            inc a
            ld (U_DATA__U_ININTERRUPT), a

.ifeq CPU_Z180
            ; On Z180 we have more than one IRQ, so we need to track of which one
            ; we arrived through. The IRQ handler sets irqvector_hw when each
            ; interrupt arrives. If we are not already handling an interrupt then
            ; we copy this into _irqvector which is the value the kernel code
            ; examines (and will not change even if reentrant interrupts arrive). 
            ; Generally the only place that irqvector_hw should be used is in 
            ; the platform_interrupt_all routine.
            .globl hw_irqvector
            .globl _irqvector
            ld a, (hw_irqvector)
            ld (_irqvector), a
.endif

            ; switch stacks
            ld (istack_switched_sp), sp
	    ; the istack is not banked (very important!)
            ld sp, #istack_top

	    call map_save

	    ld a, (0)		; save address 0 contents for checking
	    ld b, a

	    call map_kernel
	    ;
	    ; kernel_flag is in the kernel map so we need to map early, we
	    ; need to map anyway for trap_signal
	    ;
	    ld a, (_kernel_flag)
	    or a
	    push af
.ifeq PROGBASE
	    jr nz, in_kernel
            ; we're not in kernel mode, check for signals and fault
	    ld a, #0xC3
	    cp b		; should be a jump
	    jr z, nofault
	    call map_process_always; map the process
	    ld a, #0xC3		; put it back
	    ld (0), a		; write
	    call map_kernel	; restore the map
	    ld hl, #11		; SIGSEGV
	    call trap_signal	; signal the user with a fault

nofault:
in_kernel:
.endif
            ; set inint to true
            ld a, #1
            ld (_inint), a

	    ; this may task switch if not within a system call
	    ; if we switch then istack_switched_sp is out of date.
	    ; When this occurs we will exit via the resume path 
	    push af
            call _platform_interrupt
	    pop af

            xor a
            ld (_inint), a

	    pop af			; Z = in kernel
	    jr nz, in_kernel_2

	    ; On a return to user space always do a new map, we may have
	    ; changed process
	    call map_process_always
            jr int_switch
	    ; On a return from an interrupt in kernel mode restore the old
	    ; mapping as it will vary during kernel activity and the kernel
	    ; wants it put back as it was before
in_kernel_2:
	    call map_restore
int_switch:
            ld sp, (istack_switched_sp)	; stack back

            xor a
            ld (U_DATA__U_ININTERRUPT), a

            ld a, (U_DATA__U_INSYS)
            or a
            jr nz, interrupt_return

	    ; FIXME: check kernel mode flag ?
            ; we're not in kernel mode, check for signals
            call dispatch_process_signal
	    ; FIXME: we should loop for multiple signals probably

interrupt_return:
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
            ei
.ifeq CPU_Z180
            ; WRS - we could examine hw_irqvector and return with ret/reti as appropriate?
            ret
.else
            reti
.endif

;  Enter with HL being the signal to send ourself
trap_signal:
            push hl
	    ld hl, (U_DATA__U_PTAB);
            push hl
	    push af
            call _ssig
	    pop af
            pop hl
            pop hl
	    ret

;  Called from process context (hopefully)
null_handler:
	    ; kernel jump to NULL is bad
	    ld a, (U_DATA__U_INSYS)
	    or a
	    jp z, trap_illegal
	    ; user is merely not good
            ld hl, #7
            push hl
	    ld hl, (U_DATA__U_PTAB)
            push hl
	    ld hl, #10		; signal (getpid(), SIGBUS)
            rst #0x30		; syscall
            pop hl
            pop hl
            ld hl, #0
            rst #0x30		; exit



illegalmsg: .ascii "[trap_illegal]"
            .db 13, 10, 0

trap_illegal:
        ld hl, #illegalmsg
        call outstring
        call _trap_monitor

dpsmsg: .ascii "[dispsig]"
        .db 13, 10, 0


nmimsg: .ascii "[NMI]"
        .db 13,10,0
nmi_handler:
	call map_kernel
        ld hl, #nmimsg
        call outstring
        jp _trap_monitor


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

_in:
	pop hl
	pop de
	pop bc
	push bc
	push de
	push hl
	in l, (c)
	ret


;
;	Pull in the CPU specific workarounds
;

.ifeq CPU_NMOS_Z80
	.include "lowlevel-z80-nmos-banked.s"
.else
	.include "lowlevel-z80-cmos-banked.s"
.endif

;
;	These are needed by the experimental SDCC size optimisations
;
	.area _COMMONMEM

	.globl __enter, __enter_s

__enter:
	pop hl		; return address
	push ix		; save frame pointer
	ld ix, #0
	add ix, sp	; set ix to the stack frame
	jp (hl)		; and return

__enter_s:
	pop hl		; return address
	push ix		; save frame pointer
	ld ix, #0
	add ix, sp	; frame pointer
	ld e, (hl)	; size byte
	ld d, #0xFF	; always minus something..
	inc hl
	ex de, hl
	add hl, sp
	ld sp, hl
	push de
	ret

;
;	This must be in common in banked builds
;
	.globl __sdcc_call_hl

__sdcc_call_hl:
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

; The Z80 has the ldir and lddr instructions, which are perfect for implementing memmove().

_memcpy:
_memmove:
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

