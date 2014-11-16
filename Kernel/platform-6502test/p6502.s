;
;	    6502 Build testing
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_process
	    .export map_process_always
	    .export map_save
	    .export map_restore

            ; exported debugging tools
            .export _trap_monitor
            .export outchar
	    .export _di
	    .export _ei
	    .export _irqrestore

	    .import interrupt_handler
	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import unix_syscall_entry

            .include "kernel.def"
            .include "../kernel02.def"
	    .include "zeropage.inc"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

trapmsg:    .asciiz "Trapdoor: SP="
trapmsg2:   .asciiz ", PC="
tm_user_sp: .word 0

_trap_monitor:
	    sei
	    jmp _trap_monitor

_trap_reboot:
;	    lda 0xff90
;	    anda #0xfc		; map in the ROM
;	    jmp 0

_di:
	    php
	    sei			; FIXME: save old state in return to C
	    pla			; Old status
	    rts
_ei:
	    cli			; on 6502 cli enables IRQs!!!
	    rts

_irqrestore:
	    and #4		; IRQ flag
	    beq irq_on
	    cli
	    rts
irq_on:
	    sei
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
            rts

init_hardware:
            ; set system RAM size for test purposes
	    lda #1
	    sta _ramsize+1
	    lda #0
	    sta _ramsize
	    sta _procmem+1
	    lda #192
	    sta _procmem

            jsr program_vectors_k

            rts


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .segment "COMMONMEM"

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    sei
	    ;
	    ; Fixme: block copy stubs segment as well if 6509.
	    ;

	    ; our C caller will invoke us with the pointer in x,a
	    ; just pass it on
	    jsr map_process
program_vectors_k:
	    lda #<vector
	    sta $FFFE
	    lda #>vector
	    sta $FFFF
	    lda #<nmi_handler
	    sta $FFFA
	    lda #>nmi_handler
	    sta $FFFB
	    ; However tempting it may be to use BRK for system calls we
	    ; can't do this on an NMOS 6502 because the chip has brain
	    ; dead IRQ handling buts that could simply "lose" the syscall!
	    lda #JSR
	    sta syscall
	    lda #<syscall_entry
	    sta syscall+1
	    lda #>syscall_entry
	    sta syscall+2
	    jsr map_kernel
	    rts

;
;	On a fully switching system with far copies like the 6509 this is
;	all basically a no-op
;
;	On a banked setup the semantics are:
;
;	map_process_always()
;	Map the current process (ie the one with the live uarea)
;
;	map_kernel()
;	Map the kernel
;
;	map_process(pageptr [in X,A])
;	if pageptr = 0 then map_kernel
;	else map_process using the pageptr
;
;	map_save
;	save the current mapping
;
;	map_restore
;	restore the saved mapping
;
;	save/restore are used so that the kernel can play with its internal
;	banking/mappings without having to leave interrupts off all the time
;
map_process_always:
	    pha
	    lda #<U_DATA__U_PAGE
	    ldx #>U_DATA__U_PAGE
	    jsr map_process_2
	    pla
	    ret
;
;	X,A points to the map table of this process
;
map_process:
	    cmp #0
	    bne map_process_2
	    cpx
	    bne map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;
map_kernel:
	    lda #1	; for 6509 clean up any far copy ptr
	    sta 1
	    rts

; X,A holds the map table of this process
map_process_2:
	    tya
	    pha
	    sta ptr1
	    sty ptr1+1
	    ldy #0
	    lda (ptr1),y	; 4 bytes if needed
	    sta map_current
	    pla
	    tay
	    rts


;
;	Restore the indirection register on the 6509, just in case we
;	interrupted mid far copy
;
map_restore:
	    pha
	    lda saved_map
	    sta 1
	    pla
	    rts

;	Save the current mapping. For a 6509 just save the indirection
;
map_save:
	    pha
	    lda 1
	    sta saved_map
	    pla
	    rts

saved_map:  .dbyt 0
map_current: .dbyt 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
outcharw:
;	    ldb 0xffa0
;	    bitb #0x02
;	    beq outcharw
;	    sta 0xffa1
;	    puls b,pc
	    rts

;
;	Code that will live in each bank at the same address and is copied
;	there on 6509 type setups. On 6502 it may well be linked once in
;	common space
;
	.segment "STUBS"
;
;	Interrupt vector logic. Keep this in platform for the 6502 so that
;	we can use the shorter one for the CMOS chip
;
;	FIXME: spot BRK instructions and turn them into a synchronous
; SIGTRAP
;
vector:
	    pha
	    txa
	    pha
	    tya
	    pha
	    cld
	    tsx
	    inx
	    inx
	    inx
	    inx
	    lda $0100,X
	    and a, $10
;
;	FIXME: either don't care about brk or ship it somewhere like
;	kill -SIGTRAP
;
	    bne bogon
;
;	Switch to the kernel memory mapping (stack gone away)
;
	    lda 1	; Save the indirection vector
	    pha
	    lda #1	; Kernel in bank 1
	    sta 1
	    sta 0
;	The next fetch will occur from the new mapping (kernel) copy
;	Stack has gone for a walk if we were not coming from kernel
	    tsx
	    stx istack_switched_sp		; in uarea/stacks
	    ldx #0xC0
	    txs					; our istack
	    jsr interrupt_handler
	    ldx istack_switched_sp
	    txs					; entry stack
	    lda _kernel_flag
	    beq vector_um
;
;	Already in kernel (bank 1)
;
	    pla
	    sta 1				; restore the saved indirect
	    jmp irqout
vector_um:
	    lda U_DATA__U_PAGE			; may be a new task
	    sta 1				; flip to it
	    sta 0
; leap back into user context (may be a different user context to the one
; we left. Stack now valid again
	    pla					; discard saved idirect

irqout:
	    pla
	    tya
	    pla
	    txa
	    pla
	    rti


;	    X, A holds the syscall block
;	    Y holds the call code
;	    We assume the kernel is in bank 0
;
syscall_entry:
	    sei
	    sty tmp1		; for a moment
	    sta ptr1
	    stx ptr1+1
	    lda #<U_DATA__U_ARGN
	    sta ptr2
	    lda #>U_DATA__U_ARGN
	    sta ptr2+1
	    lda #1
	    sta 1		; magic far copy hackery

	    ldx #0
	    txy
copy_args:  lda (ptr1), x	; copy the arguments from current bank
	    sta (ptr2), y	; will write into bank 1
	    inx
	    iny
	    cpx #8
	    bne copy_args
	    ldy tmp1		; syscall code
	    ;
	    ; Now we need to bank and stack switch
	    ;
	    tsx
	    lda #1
	    sta 0		; kernel banks
	    sta 1
;
;	We are now suddenely in the kernel copy of this, and our stack is
;	missing in action. Access to userspace is not available
;
;	On a 6509 this also means our C stack is missing in action, which
;	is quite convenient as it saves us a save and restore it. On other
;	CPUs it might be a little less convenient
;
	    stx U_DATA__U_SP
	    ldx #0
;
;	FIXME: how to handle IRQ division of stack ???
;
	    txs			; Switch to the working stack
;
;	Set up the C stack
;
	    lda #<kstack_top
	    sta sp
	    lda #>kstack_top
	    sta sp+1

	    cli
;
;	Caution: We may enter here and context switch and another task
;	exit via its own syscall returning in its own memory context.
;
;	Don't assume anything we stored statically *except* the uarea
;	will be different. The uarea is banked in and out (or copied in
;	more awkward systems).
;
	    jsr unix_syscall_entry

	    sei
;
;	Correct the system stack
;
	    ldx U_DATA__U_SP
	    txs
	    lda U_DATA__U_PAGE	; our bank
	    sta 1		; use our bank for sta (ptr), y
;
;	Copy the return data over
;
	   ldy #8		; write them after the argument block
;
;	As we are a user process and making a syscall our ptr1 was saved
;	and banked out. If you had common memory this would be uglier but
; 	then you've got somewhere else to put the pointer right !
;
	   lda U_DATA__U_ERROR
	   sta (ptr1), y
	   iny
	   lda U_DATA__U_ERROR+1
	   sta (ptr1),y
	   iny
	   lda U_DATA__U_RETVAL
	   sta (ptr1),y
	   iny
	   lda U_DATA__U_RETVAL+1
	   sta (ptr1), y
;	FIXME Also copy over needed signal information (vector, pending stuff)
;
	   lda 1		; The sta,y redirection bank
	   sta 0		; becomes our execution bank
;
;	We just teleported back to the copy of this code in the
;	user process bank
;
;	Our C stack is valid as we simply bounced out of it for the 6509.
;
;
;	FIXME: do signal dispatch - this will need C stack fixing, and
;	basically signal dispatch is __interrupt.
;

	    rts
