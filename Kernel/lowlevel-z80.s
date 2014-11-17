;
;	Common elements of low level interrupt and other handling. We
; collect this here to minimise the amount of platform specific gloop
; involved in a port
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
        .globl map_process
        .globl map_save
        .globl map_restore
	.globl outchar
	.globl _kernel_flag
	.globl _inint
	.globl _platform_interrupt
	.globl platform_interrupt_all

        .module syscall

        ; exported symbols
        .globl unix_syscall_entry
	.globl _chksigs
	.globl null_handler
	.globl _system_tick_counter
	.globl unix_syscall_entry
	.globl dispatch_process_signal
        .globl _doexec
        .globl trap_illegal
	.globl nmi_handler
	.globl interrupt_handler
	.globl _di
	.globl _irqrestore

        ; imported symbols
        .globl _trap_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
        .globl dispatch_process_signal
	.globl istack_switched_sp
	.globl istack_top
	.globl _ssig

        .include "platform/kernel.def"
        .include "kernel.def"

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

	; make sure the interrupt logic knows we are in kernel mode
	ld a, #1
	ld (_kernel_flag), a

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

        ; re-enable interrupts
        ei

        ; now pass control to C
        call _unix_syscall

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
	call _chksigs
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

            ld hl, (_system_tick_counter)
            inc hl
            ld (_system_tick_counter), hl

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
            ; set inint to true
            ld a, #1
            ld (_inint), a

	    ; this may task switch if not within a system call
	    ; if we switch then istack_switched_sp is out of date.
	    ; When this occurs we will exit via the resume path 
            call _platform_interrupt

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
            reti

;  Enter with HL being the signal to send ourself
trap_signal:
            push hl
	    ld hl, (U_DATA__U_PTAB);
            push hl
            call _ssig
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


		; IRQ helpers, in common as they may get used by common C
		; code (and are tiny)

_di:		ld a, i
		push af
		pop hl
		di
		ret

_irqrestore:	pop hl		; sdcc needs to get register arg passing
		pop af		; so badly
		jp po, was_di
		ei
		jr irqres_out
was_di:		di
irqres_out:	push af
		jp (hl)




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

outhl:  ; prints HL in hex. Destroys AF.
        ld a, h
        call outcharhex
        ld a, l
        call outcharhex
        ret

outbc:  ; prints BC in hex. Destroys AF.
        ld a, b
        call outcharhex
        ld a, c
        call outcharhex
        ret

outde:  ; prints DE in hex. Destroys AF.
        ld a, d
        call outcharhex
        ld a, e
        call outcharhex
        ret

; print the byte in A as a two-character hex value
outcharhex:
        push bc
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
        pop bc
        ret

; print the nibble in the low four bits of A
outnibble:
        and #0x0f ; mask off low four bits
        cp #10
        jr c, numeral ; less than 10?
        add a, #0x07 ; start at 'A' (10+7+0x30=0x41='A')
numeral:add a, #0x30 ; start at '0' (0x30='0')
        call outchar
        ret

