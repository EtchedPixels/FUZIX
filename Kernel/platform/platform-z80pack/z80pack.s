;
;	Z80Pack hardware support
;
;
;	This goes straight after udata for common.
;


            .module z80pack

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl plt_interrupt_all

	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_proc
	    .globl map_proc_di
	    .globl map_proc_always
	    .globl map_proc_always_di
	    .globl map_proc_a
	    .globl map_save_kernel
	    .globl map_restore

	    .globl _fd_bankcmd

	    .globl _int_disabled

	    .globl _plt_reboot

            ; exported debugging tools
            .globl _plt_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler
	    .globl _doexit
	    .globl kstack_top
	    .globl _panic
	    .globl mmu_irq_ret
	    .globl _need_resched
	    .globl _ssig

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_plt_monitor:
	    ld a, #128
	    out (29), a
plt_interrupt_all:
	    ret

_plt_reboot:
	    ld a, #1
	    out (29), a

;
;	We need the right bank present when we cause the transfer
;
_fd_bankcmd:pop de		; return
	    pop bc		; command
	    pop hl		; bank
	    push hl
	    push bc
	    push de		; fix stack
	    ld a, (_int_disabled)
	    di
	    push af		; save DI state
	    call map_proc_di	; HL alread holds our bank
	    ld a, c		; issue the command
	    out (13), a		;
	    call map_kernel_di	; return to kernel mapping
	    pop af
	    or a
	    ret nz
	    ei
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, #240			; 240 * 256 bytes (60K)
	    out (22), a			; set up memory banking
	    ld a, #8	
	    out (20), a			; 8 segments
            ret

init_hardware:
            ; set system RAM size
            ld hl, #484
            ld (_ramsize), hl
            ld hl, #(484-64)		; 64K for kernel
            ld (_procmem), hl

	    ld a, #1
	    out (27), a			; 100Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld a, #0xfe			; Use FEFF (currently free)
	    ld i, a
            im 2 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_int_disabled:
	    .byte 1

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_proc

            ; write zeroes across all vectors
            ld hl, #0
            ld de, #1
            ld bc, #0x007f ; program first 0x80 bytes only
            ld (hl), #0x00
            ldir

            ; now install the interrupt vector at 0xFEFF
            ld hl, #interrupt_handler
            ld (0xFEFF), hl
            ; now install the interrupt vector at 0xFEFF
            ld hl, #interrupt_handler
            ld (0xFEFF), hl

	    ld a,#0xC3		; JP
            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ; Set vector for jump to NULL
            ld (0x0000), a   
            ld hl, #null_handler  ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; our platform has a "true" common area, if it did not we would
	    ; need to copy the "common" code into the common area of the new
	    ; process.

	    ; falls through

            ; put the paging back as it was -- we're in kernel mode so this is predictable
map_kernel:
map_kernel_di:
map_kernel_restore:
	    push af
	    xor a
	    out (21), a
	    pop af
            ret
map_proc:
map_proc_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_proc_a:
	    out (21), a
            ret
map_proc_always:
map_proc_always_di:
	    push af
	    ld a, (_udata + U_DATA__U_PAGE)
	    out (21), a
	    pop af
	    ret
map_save_kernel:
	    push af
	    in a, (21)
	    ld (map_store), a
	    xor a
	    out (21),a
	    pop af
	    ret	    
map_restore:
	    push af
	    ld a, (map_store)
	    out (21), a
	    pop af
	    ret	    
map_store:
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x01), a
            ret

;
;	The entry logic is a bit scary. We want to make sure that we
;	don't get tricked into anything bad by messed up callers.
;
;	At the point we are called the push hl and call to us might have
;	gone onto a dud stack, but if so that is ok as we won't be returning
;
mmu_kernel:
	    push af
	    push hl
	    ld hl,#0
	    add hl,sp
	    ld a,h
	    or a			; 00xx is bad
	    jr z, badstack
	    cp #0xF0
	    jr nc, badstack		; Fxxx is bad
	    in a,(23)
	    bit 7,a			; Tripped MMU is bad if user
	    jr nz, badstackifu
do_mmu_kernel:
	    xor a			; Switch MMU off
	    out (23),a
	    pop hl
	    pop af
	    ret
;
;	We have been called with SP pointing into la-la land. That means
;	something bad has happened to our process (or the kernel)
;
badstack:
	    ld a, (_udata + U_DATA__U_INSYS)
	    or a
	    jr nz, badbadstack
	    ld a, (_udata + U_DATA__U_ININTERRUPT)
	    or a
	    jr nz, badbadstack
	    ;
	    ;	Ok we are in user mode, this is less bad.
	    ;	Fake a sigkill
	    ;
badstack_do:
	    ld sp, #kstack_top		; Our syscall stack
	    xor a
	    out (23),a			; MMU off
	    call map_kernel		; So we can _doexit
	    ld hl,#9			; SIGKILL
	    push hl
;	    ld a,#'@'
;	    call outchar
	    call _doexit		; Will not return	    
	    ld hl,#zombieapocalypse
	    push hl
	    call _panic
zombieapocalypse:
	    .asciz "ZombAp"

badbadstack:
	    ld hl,#badbadstack_msg
	    push hl
	    call _panic

badstackifu:
	    ld a, (_udata + U_DATA__U_INSYS)
	    or a
	    jr nz, do_mmu_kernel
	    ld a, (_udata + U_DATA__U_ININTERRUPT)
	    or a
	    jr nz, do_mmu_kernel
	    jr badstack_do

;
;	IRQ version - we need different error handling as we can't just
;	exit and have the IRQ vanish (we'd survive fine but the IRQ wouldn't
;	get processed).
;
mmu_kernel_irq:
	    ld hl,#0
	    add hl,sp
	    ld a,h
	    or a
	    jr z, badstackirq
	    inc a
	    jr z,badstackirq
	    in a,(23)
	    bit 7,a
	    jr nz, badstackirqifu
do_mmu_kernel_irq:
	    in a,(23)
	    ld l,a
	    xor a
	    out (23),a
	    ld a,l
	    ld (mmusave),a
	    jp mmu_irq_ret

	    ld a, (_udata + U_DATA__U_INSYS)
	    or a
	    ld a, (_udata + U_DATA__U_ININTERRUPT)
	    or a
badstackirq:
	    ld a, (_udata + U_DATA__U_INSYS)
	    or a
	    jr nz, badbadstack_irq
	    ld a, (_udata + U_DATA__U_ININTERRUPT)
	    or a
	    jr nz, badbadstack_irq
badstack_doirq:
	    ;
	    ;	If we get here we *are* interrupted from user space and
	    ;	thus we can safely use map_save/map_restore
	    ;
	    ; Put the stack somewhere that will be safe - kstack will do
	    ; The user stack won't do as we're going to switch to kernel
	    ; mappings
	    ld sp,#kstack_top
	    xor a
	    out (23),a			; MMU off so we can do our job
;	    ld a,#'!'
;	    call outchar
	    call map_save_kernel
	    ld hl,#9
	    push hl
	    ld hl,(_udata + U_DATA__U_PTAB)
	    push hl
	    call _ssig
	    pop hl
	    pop hl
	    ld a,#1
	    ld (_need_resched),a
	    call map_restore
	    ;
	    ; Ok this looks pretty wild but the idea is that the stack we
	    ; came in on could be completely hosed so we just need somewhere
	    ; in user memory to scribble freely. The pre-emption path will
	    ; kill us before we return to userland and use that stack for
	    ; anything non-kernel
	    ;
	    ld sp,#0x8000
	    jp mmu_irq_ret
	    ; This will complete the IRQ and then hit preemption at which
	    ; point it'll call switchout, chksigs and vanish forever
badstackirqifu:
	    ld a, (_udata + U_DATA__U_INSYS)
	    or a
	    jr nz, do_mmu_kernel_irq
	    ld a, (_udata + U_DATA__U_ININTERRUPT)
	    or a
	    jr nz, do_mmu_kernel_irq
	    jr badstack_doirq

badbadstack_irq:
	    ld hl,#badbadstackirq_msg
	    push hl
	    call _panic

badbadstackirq_msg:
	    .ascii 'IRQ:'
badbadstack_msg:
	    .asciz 'MMU trap/bad stack'


;
;	This side is easy. We are coming from a sane context (hopefully).
;	MMU on, clear flag.
;
mmu_restore_irq:
	    ld a,(mmusave)
	    and #0x01
	    out (23),a
	    ret
mmu_user:
	    ld a,#1
	    out (23),a
	    ret
mmusave:
	    .byte 0
