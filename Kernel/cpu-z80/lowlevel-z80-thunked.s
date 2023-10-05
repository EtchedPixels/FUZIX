;
;	Common elements of low level interrupt and other handling. We
; collect this here to minimise the amount of platform specific gloop
; involved in a port
;
; This version is intended for those awkward platforms with 32K/32K banking
; or 64K whole memory bank flipping.
;
; Some features are controlled by Z80_TYPE which should be declared in
; platform/kernel.def as one of the following values:
;     0   CMOS Z80
;     1   NMOS Z80
;
;	Based upon code (C) 2013 William R Sowerbutts
;

	.module lowlevel-thunked

	; debugging aids
	.globl outcharhex
	.globl outbc, outde, outhl
	.globl outnewline
	.globl outstring
	.globl outstringhex
	.globl outnibble

	; platform provided functions
	.globl map_kernel_low
	.globl map_user_low
	.globl map_page_low
        .globl map_save_low
        .globl map_restore_low
	.globl outchar
	.globl _plt_interrupt
	.globl _need_resched
	.globl _plt_switchout
	.globl _plt_doexec

        ; exported symbols
	.globl unix_syscall_entry
        .globl _doexec
	.globl nmi_handler
	.globl interrupt_handler
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

        ; imported symbols
	.globl _chksigs
	.globl _int_disabled
        .globl _plt_monitor
        .globl _unix_syscall
        .globl outstring
        .globl kstack_top
	.globl istack_switched_sp
	.globl istack_top
	.globl _udata
	.globl syscall_platform

        .include "build/kernel.def"
        .include "kernel-z80.def"

	HIGH
;
;	Execve also needs a platform helper for 32/32K
;
;	Runs a low memory stub helper in the user bank with
;	HL = start address, IY = relocation base
;	Helper must re-enable interrupts
;
_doexec:
	di
	call map_user_low
	xor a
	ld (_udata + U_DATA__U_INSYS),a
	pop bc
	pop de			; start address
	ld hl,(_udata + U_DATA__U_ISP)
	ld sp,hl
	ex de,hl
	ld d,h
	ld e,#0
	ld a,(_udata + U_DATA__U_PAGE+HIGHPAGE) ; pass high page to trampoline
	jp _plt_doexec	; jump into the low memory stub

;
;	This is the entry point from the platform wrapper. When we hit this
;	our stack is above 32K and the upper 32K of kernel space is mapped
;	and the low 32K of user space.
;
;	The arguments are in BC DE HL IX and the syscall number in A
;	The caller has saved IX for us and A BC DE HL don't matter
;	The caller has placed us on kstack top and already saved the user sp
;
;	Our return is
;	A - page to map high
;	DE - retval
;	H - signal or 0
;	L - errno
;	BC - signal vector
;
;	We enter and exit with interrupts off, we will enable interurpts in
;	the mean time. Due to fork() we may exit twice from one call, and
; 	due to exiting we may never leave!
;
unix_syscall_entry:
	; Start by saving the arguments. That frees up all our registers
	ld (_udata + U_DATA__U_CALLNO),a	
	ld (_udata + U_DATA__U_ARGN),bc
	ld (_udata + U_DATA__U_ARGN1),de
	ld (_udata + U_DATA__U_ARGN2),ix
	ld (_udata + U_DATA__U_ARGN3),hl
	; Stack the alternate registers
	exx
	push bc
	push de
	push hl
	ex af,af'
	push af
	; Tell the pre-emption code we are in kernel right now
	ld a,#1
	ld (_udata + U_DATA__U_INSYS),a
	; Make the kernel appear
	call map_kernel_low
	; Call into the kernel with interrupts on
	ei
	call _unix_syscall
	; Unmap the kernel low 32K
	di
	call map_user_low
	; We are now not in kernel
	xor a
	ld (_udata + U_DATA__U_INSYS),a
	; Recover the return data
	ld hl, (_udata + U_DATA__U_ERROR)
	ld de, (_udata + U_DATA__U_RETVAL)
	ld a, (_udata + U_DATA__U_CURSIG)
	or a
	; Keep the hot path inline
	jr nz, signal_path
no_signal:
	; No signal pending
	ld h,#0
syscall_return:
	; restore the alternate registers
	pop af
	ex af,af'
	exx
	pop hl
	pop de
	pop bc
	exx
	; Return page for caller (may not be the page we came in on if we
	; swapped
	ld a,(_udata + U_DATA__U_PAGE+HIGHPAGE)
	ret
signal_path:
	ld h,a		; save signal number
	push hl
	; Get the signal vector and zero it
	add a,a
	ld hl,#_udata + U_DATA__U_SIGVEC
	ld c,a
	xor a
	ld (_udata + U_DATA__U_CURSIG),a
	ld b,a
	add hl,bc
	ld c,(hl)
	ld (hl),a	; clear the vector (b = 0)
	inc hl
	ld b,(hl)
	ld (hl),a
	pop hl
	ld a,b
	or c
	jr z, no_signal	; cleared under us (can this occur ??)
	jr syscall_return



nmimsg: .ascii "[NMI]"
        .db 13,10,0

nmi_handler:
	call map_kernel_low
        ld hl, #nmimsg
traphl:
        call outstring
        call _plt_monitor

	HIGH
;
;	The stub caller has already saved AF DE HL, mapped the kernel high
;	and switched to the istack as well as saving the old sp in
;	istack_switched_sp
;
interrupt_handler:
	call map_save_low	; save old and map kernel
	ld a,#1
	ld (_udata + U_DATA__U_ININTERRUPT),a
	ld (_int_disabled),a
	call _plt_interrupt
	ld a,(_need_resched)
	or a
	jr nz, preemption
	ld hl,#intreti
	push hl
	reti			; Generate M1 RETI cycles to clear
				; any interrupt controller
intreti:di
	xor a
	ld (_udata + U_DATA__U_ININTERRUPT),a
	
	; Are we returning to kernel ?
	ld a, (_udata + U_DATA__U_INSYS)
	or a
	jr nz, interrupt_kernel
intsig:
	call map_user_low
	ld a,(_udata + U_DATA__U_CURSIG)
	or a
	jr nz, interrupt_sig
	; Nothing special to do
no_sig:
	xor a
	ld (_int_disabled),a
	ld e,a
	ld a,(_udata + U_DATA__U_PAGE+HIGHPAGE)
intret:
	ret

interrupt_kernel:
	call map_restore_low
	xor a
	ld (_int_disabled),a
	ld e,a
	jr intret
;
;	Signal return. Get the vector, clear the vector and queue, then pass
;	the right info back
;
interrupt_sig:
	ld e,a
	xor a
	ld (_int_disabled),a
	ld d,a
	ld c,a
	ld (_udata + U_DATA__U_CURSIG),a
	ld hl,#_udata + U_DATA__U_SIGVEC
	add hl,de
	add hl,de
	ld e,(hl)
	ld (hl),a
	inc hl
	ld d,(hl)
	ld (hl),a
	ld a,d
	or e
	jr z, no_sig
	ex de,hl
	ld e,c
	jr intret

;
;	The horrible case - task switching time
;
;	Everything important is on the user stack, except that there is
;	a vital stubs return address top of our current stack
;
;	We move the return address onto the kstack, switch to the kstack
;	and then park ourselves in plt_switchout. On our return the
;	istack has been lost but our kstack (private to us) still has the
;	needed return address to the stubs and we can unwind everything
;	via that stack.
;
.globl preemption

preemption:
	pop de				; return address to stubs
	xor a
	ld (_need_resched),a		; we are doing it thank you
	ld hl,(istack_switched_sp)
	ld (_udata + U_DATA__U_SYSCALL_SP), hl	; save the stack save
	ld sp, #kstack_top		; free because we are not pre-empted
					; during a system call
	push de				; return address onto our kstack
					; istack will be lost
	ld hl,#intret2
	push hl
	reti				; reti M1 cycle for IM2
intret2:
	di
	; Fake being in a syscall
	ld a,#1
	ld (_udata + U_DATA__U_INSYS),a
	call _chksigs
	ld hl, (_udata + U_DATA__U_PTAB)
	ld a,#P_RUNNING
	cp (hl)
	jr nz, not_running
	ld (hl), #P_READY
	inc hl
	set PFL_BATCH,(hl)
not_running:
	; Give up the CPU
	; FIXME: can we get here on timers when only one thing is running
	; and we don't need to pre-empt ????? Is this more generally busted
	; ? It is .... review timer interrupt logic... FIXME
	call _plt_switchout
	; Undo the fakery
	xor a
	ld (_udata + U_DATA__U_ININTERRUPT),a
	ld (_udata + U_DATA__U_INSYS),a
	; The istack was lost but that is ok as we saved the return onto the
	; kernel stack, so when we finally ret we end up in the right place
	ld sp,#kstack_top - 2		; saved return address

	ld hl,(_udata + U_DATA__U_SYSCALL_SP)
	ld (istack_switched_sp),hl

	; Now continue on the interrupt return path
	; looking for signals
	jr intsig

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
	jp syscall_platform	; different entry rules for thunked
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
