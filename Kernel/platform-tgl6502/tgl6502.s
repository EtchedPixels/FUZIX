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

	    .export _unix_syscall_i
	    .export _platform_interrupt_i
	    .export platform_doexec

            ; exported debugging tools
            .export _trap_monitor
            .export outchar
	    .export _di
	    .export _ei
	    .export _irqrestore
	    .export nmi_trap
	    .export vector

	    .import interrupt_handler
	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import unix_syscall_entry
	    .import kstack_top
	    .import istack_switched_sp
	    .import istack_top
	    .import _unix_syscall
	    .import _platform_interrupt
	    .import _kernel_flag

            .include "kernel.def"
            .include "../kernel02.def"
	    .include "zeropage.inc"

;
;	syscall is jsr [$00fe]
;
syscall	     =  $FE
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

nmi_trap:
_trap_monitor:
	    sei
	    jmp _trap_monitor

_trap_reboot:
	    jmp _trap_reboot	; FIXME: original ROM map and jmp

_di:
	    php
	    sei			; Save old state in return to C
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
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
            rts

init_hardware:
            ; set system RAM size for test purposes
	    lda #0
	    sta _ramsize+1
	    sta _procmem+1
	    lda #128
	    sta _ramsize
	    lda #120
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
	    lda #<syscall_entry
	    sta syscall
	    lda #>syscall_entry
	    sta syscall+1
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
	    rts
;
;	X,A points to the map table of this process
;
map_process:
	    cmp #0
	    bne map_process_2
	    cpx #0
	    bne map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 40/42/43/.... (41 has the reset vectors in so
;	we avoid it. Later we'll be clever and stuff _DISCARD and the copy
;	blocks there or something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel:
	    pha
				; Common is left untouched as is ZP and S
	    lda #$01		; Kernel RAM at 0x2000-0x3FFF
	    sta $FF8B		; 
	    ldx #$40		; Kernel ROM at 0x4000-0xFFFF
	    stx $FF8C
	    inx			; Skip second bank
	    inx
	    stx $FF8D
	    inx
	    stx $FF8E
	    inx
	    stx $FF8F
	    inx
	    stx $FF90
	    inx
	    stx $FF91
	    pla
	    rts

; X,A holds the map table of this process
map_process_2:
	    sta ptr1
	    tya
	    pha
	    sty ptr1+1
	    ldy #0
	    lda (ptr1),y	; 4 bytes if needed
	    jsr restore_bits
	    pla
	    tay
	    rts


;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 6 byte save/restore  FIXME
;
map_restore:
	    pha
	    txa
	    pha
	    ldx saved_map
	    jsr restore_bits
	    pla
	    tax
	    pla
	    rts

;
;	Restore all but the zero page and the top of memory map for this
;	process. The ZP means 0-1FFF are not mapped the way the core
;	kernel expects. We need to handle that in our user copiers
;
restore_bits:
	    inx			; We don't want to play with the ZP
	    stx $FF8B		; 2000
	    inx
	    stx $FF8C		; 4000
	    inx
	    stx $FF8D		; 6000
	    inx
	    stx $FF8E		; 8000
	    inx
	    stx $FF8F		; A000
	    inx
	    stx $FF90		; C000
				; E000 we don't have enough RAM for until
				; we go to proper banking
	    rts

;	Save the current mapping.
;
;	FIXME: need to copy the 8 byte area
;
map_save:
	    pha
	    sta saved_map
	    pla
	    rts

saved_map:  .byte 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    pha
outcharw:
	    lda $FF01
	    and #4
	    beq outcharw
	    pla
	    sta $FF03
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
vector:
	    pha
	    txa
	    pha
	    tya
	    pha
	    cld
;
;	Save the old stack ptr
;
	    lda sp				; put the C stack 
	    pha					; on the 6502 stack
	    lda sp+1
	    pha
	    tsx					; and save the 6502 stack ptr
	    stx istack_switched_sp		; in uarea/stacks

;
;	Hope the user hasn't gone over 192 bytes of 6502 stack !
;
	    ldx #$40				; guess at reserving some istack
	    txs					; our istack
;
;	Configure the C stack to the i stack
;
	    lda #<istack_top
	    sta sp
	    lda #>istack_top
	    sta sp+1
	    jsr interrupt_handler
;
;	Reload the previous value into the stack ptr
;
	    ldx istack_switched_sp
	    txs					; recover 6502 stack
	    pla					; recover C stack
	    sta sp+1				; from 6502 stack
	    pla
	    sta sp+1
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
	    sty U_DATA__U_CALLNO
	    pha
	    txa
	    pha
	    sta ptr1
	    stx ptr1+1
	    lda #<U_DATA__U_ARGN
	    sta ptr2
	    lda #>U_DATA__U_ARGN
	    sta ptr2+1

	    ldy #0

copy_args:  lda (ptr1),y	; copy the arguments from current bank
	    sta (ptr2),y	; will write into bank 1
	    iny
	    cpy #8
	    bne copy_args
	    ;
	    ; Now we need to bank and stack switch
	    ;
	    lda sp
	    pha
	    lda sp+1
	    pha
	    tsx
	    stx U_DATA__U_SP
;
;	We try and divide our previous stack resource up between user
;	and kernel. It's not clear if we should do this, copy the stack
;	or do something clever I've not thought of yet. Possibly we should
;	see if there is enough stack and if not copy and screw about ?
;
	    ldx #$80
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
;
;	From that recover the C stack and the syscall buf ptr
;
	    pla
	    sta sp+1
	    pla
	    sta sp
	    pla
	    sta ptr1+1
	    pla
	    sta ptr1+1
;
;	Copy the return data over
;
	    ldy #8		; write them after the argument block
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
;
;	FIXME: do signal dispatch - this will need C stack fixing, and
;	basically signal dispatch is __interrupt.
;

	    rts


platform_doexec:
	    jmp $2000		; FIXME: should jump to argument really

;
;	Straight jumps no funny banking issues
;
_unix_syscall_i:
	    jmp _unix_syscall
_platform_interrupt_i:
	    jmp _platform_interrupt

	    .segment "ZEROPAGE"

zp_intvec:	.byte 0
