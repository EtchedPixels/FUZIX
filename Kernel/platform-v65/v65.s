;
;	    v65 platform functions
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
            .export _platform_monitor
	    .export _platform_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore
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
	    .import stash_zp
	    .import pushax
	    .import _chksigs
	    .import _platform_switchout
	    .import _need_resched
	    .import _udata

	    .import outcharhex
	    .import outxa
	    .import incaxy

	    .import _create_init_common

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

_platform_monitor:
	    jmp _platform_monitor

_platform_reboot:
	    lda #$A5
	    sta $FE40		; Off
	    jmp _platform_reboot	; FIXME: original ROM map and jmp

___hard_di:
	    php
	    sei			; Save old state in return to C
	    pla			; Old status
	    rts
___hard_ei:
	    cli			; on 6502 cli enables IRQs!!!
	    rts

___hard_irqrestore:
	    and #4		; IRQ flag
	    bne irq_on
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
	    ; copy the low 8K from bank 0 to bank 8 so we can keep the
	    ; simple model of 8-15 process 16-24 process ....
	    ; bank 0 is effectively spare at this point and can be
	    ; reused.
	    jsr _create_init_common
	    lda #$08
	    sta $FE00
            rts

init_hardware:
            ; set system RAM size for test purposes
	    lda #0
	    sta _ramsize
	    lda #4
	    sta _ramsize+1
	    lda #192
	    sta _procmem
	    lda #3
	    sta _procmem+1
            jmp program_vectors_k

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .segment "COMMONMEM"

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    sei
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
	    ; dead IRQ handling bits that could simply "lose" the syscall!
	    lda #<syscall_entry
	    sta syscall
	    lda #>syscall_entry
	    sta syscall+1
	    jmp map_kernel

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
;	ptr1 and tmp1 may be destroyed by these methods, but no other
;	temporaries.
;
map_process_always:
	    pha
	    txa
	    pha
	    ldx _udata + U_DATA__U_PAGE
	    jsr map_bank_i
	    pla
	    tax
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
	    txa
	    pha
				; Common is left untouched as is ZP and S
	    ldx #$01		; Kernel RAM at 0x2000-0x3FFF
	    jsr map_bank
	    pla
	    tax
	    pla
	    rts

;
;	Entry point to map a linear bank range
;
map_bank_i:			; We are not mapping the first user page yet
	    inx
map_bank:
	    stx $FE01
	    inx
	    stx $FE02
	    inx
	    stx $FE03
	    inx
	    stx $FE04
	    inx
	    stx $FE05
	    inx
	    stx $FE06
;	    inx			; We don't use bank 7 copies yet
;	    stx $FE07
	    rts

; X,A holds the map table of this process
map_process_2:
	    sta ptr1
	    stx ptr1+1
	    tya
	    pha
	    ldy #0
	    lda (ptr1),y	; 4 bytes if needed
	    tax
	    pla
	    tay
	    jmp map_bank_i


;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 6 byte save/restore if we do clever stuff in future
;
map_restore:
	    pha
	    txa
	    pha
	    ldx saved_map	; First bank we skip half of
	    jsr map_bank
	    pla
	    tax
	    pla
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save:
	    pha
	    lda $FE01
	    sta saved_map
	    pla
	    rts

saved_map:  .byte 0, 0, 0, 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    sta $FE20
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
;	Q: do we want to spot brk() instructions and signal them ?
;
;
;	Save the old stack ptr
;
	    jsr stash_zp			; Save zero page bits
	    tsx					; and save the 6502 stack ptr
	    stx istack_switched_sp		; in uarea/stacks
;
;	Hope the user hasn't used all the CPU stack
;
;	FIXME: we should check here if S is too low and if so set it high
;	and deliver SIGKILL
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
	    jsr stash_zp			; restore zero page bits

;
;	TODO: pre-emption
;
	    lda _need_resched
	    beq no_preempt

	    lda	#0
	    sta _need_resched

	    ; Switch to our kernel stack which is free as we don't preempt
	    ; from kernel space
	    tsx
	    stx	_udata + U_DATA__U_SYSCALL_SP
	    ldx #kstack_top
	    txs

	    ;
	    ; TODO: do we need to sort the C stack or is it sufficient to
	    ; just do the 6502 stack at this point since we have a valid
	    ; C stack.. the IRQ one for chksigs to run, while the
	    ; platform_switchout code is asm only as we switch in ?
	    ;
	    lda	#1
	    sta _udata + U_DATA__U_INSYS
	    jsr	_chksigs
	    ; Mark ourselves Ready not Running
	    ldx _udata + U_DATA__U_PTAB
	    lda	#P_READY
	    sta a:P_TAB__P_STATUS_OFFSET,x
	    ; Switch out .. we will re-appear at some point
	    lda	_udata + U_DATA__U_PTAB
	    ldx _udata + U_DATA__U_PTAB+1
	    jsr _platform_switchout
	    ;
	    ; Now return to the old state having woken back up
	    ; Clear insys, move back onto the interrupt stack
	    ;
	    lda #0
	    sta _udata + U_DATA__U_INSYS
	    ldx #<_udata + U_DATA__U_SYSCALL_SP
	    txs
	    ;
	    ; Check what signals are now waiting for us
	    ;
no_preempt:
;
;	Signal handling on 6502 is foul as the C stack may be inconsistent
;	during an IRQ. We push a new complete rti frame below the official
;	one, along with a vector and the signal number. The glue in the
;	app is expected to switch to a signal stack or similar, pop the
;	values, invoke the signal handler and then return.
;
	    lda _udata + U_DATA__U_CURSIG
	    beq irqout
	    tay
	    tsx
	    txa
	    sec
	    sbc #6			; move down past the existing rti
	    tax
	    txs
	    lda #>irqout
	    pha
	    lda #<irqout
	    pha				; stack a return vector
	    tya
	    pha				; stack signal number
	    ldx #0
	    stx _udata + U_DATA__U_CURSIG
	    asl a
	    tay
	    lda _udata + U_DATA__U_SIGVEC,y	; Our vector (low)
	    pha				; stack half of vector
	    lda _udata + U_DATA__U_SIGVEC+1,y	; High half
	    pha				; stack rest of vector
	    txa
	    sta _udata + U_DATA__U_SIGVEC,y	; Wipe the vector
	    sta _udata + U_DATA__U_SIGVEC+1,y
	    lda #<PROGLOAD + 20
	    pha
	    lda #>PROGLOAD + 20
	    lda #0
	    pha				; dummy flags, with irq enable
	    rti	    			; return on the fake frame
					; if the handler returns
					; rather than doing a longjmp
					; we'll end up at irqout and pop the
					; real frame
irqout:
	    pla
	    tay
	    pla
	    tax
	    pla
	    rti
;
;	    sp/sp+1 are the C stack of the userspace
;	    with the syscall number in X
;	    Y indicates the number of bytes of argument
;
syscall_entry:
	    php
	    sei
	    cld

	    stx _udata + U_DATA__U_CALLNO

	    ; No arguments - skip all the copying and stack bits
	    cpy #0
	    beq noargs

	    ; Remove the arguments. This is fine as by the time we go back
	    ; to the user stack we'll have finished with them
	    lda sp
	    sta ptr1
	    ldx sp+1
	    stx ptr1+1
	    jsr incaxy
	    sta sp
	    stx sp+1

	    ;
	    ;	We copy the arguments but need to deal with the compiler
	    ;   stacking in the reverse order. At this point ptr1 points
	    ;	to the last byte of the arguments (first argument). We go
	    ;	down the stack copying words up the argument list.
	    ;
	    ldx #0
copy_args:
	    dey
	    lda (ptr1),y		; copy the arguments over
	    sta _udata + U_DATA__U_ARGN+1,x
	    dey
	    lda (ptr1),y
	    sta _udata + U_DATA__U_ARGN,x
            inx
	    inx
	    cpy #0
	    bne copy_args
noargs:
	    ;
	    ; Now we need to stack switch. Save the adjusted stack we want
	    ; for return
	    ;
	    lda sp
	    pha
	    lda sp+1
	    pha
	    tsx
	    stx _udata + U_DATA__U_SYSCALL_SP
;
;	We save a copy of the high byte of sp here as we may need it to get
;	the brk() syscall right.
;
	    sta _udata + U_DATA__U_SYSCALL_SP + 1
;
;
;	FIXME: we should check here if there is enough 6502 stack left
;	and if so either copy and switch stacks or kill the process
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
	    ldx _udata + U_DATA__U_SYSCALL_SP
	    txs
;
;	From that recover the C stack and the syscall buf ptr
;
	    pla
	    sta sp+1
	    pla
	    sta sp
	    lda _udata + U_DATA__U_CURSIG
	    beq syscout
	    tay

	    tsx				; Move past existing return stack
	    dex
	    dex
	    dex
	    txs

	    ;
	    ;	The signal handler might make syscalls so we need to get
	    ;	our return saved and return the right value!
	    ;
	    lda _udata + U_DATA__U_ERROR
	    pha
	    lda _udata + U_DATA__U_RETVAL
	    pha
	    lda _udata + U_DATA__U_RETVAL+1
	    pha
	    lda #>sigret		; Return address
	    pha
	    lda #<sigret
	    pha

	    tya
	    pha				; signal
	    ldx #0
	    stx _udata + U_DATA__U_CURSIG
	    asl a
	    tay
	    lda _udata + U_DATA__U_SIGVEC,y	; Our vector
	    pha
	    lda _udata + U_DATA__U_SIGVEC+1,y
	    pha
	    txa
	    sta _udata + U_DATA__U_SIGVEC,y	; Wipe the vector
	    sta _udata + U_DATA__U_SIGVEC+1,y

	    ; Invoke the helper with signal and vector stacked
	    ; it will then return to syscout and recover the original
	    ; frame. If the handler made syscalls then
	    jmp (PROGLOAD + 20)

	    ;
	    ; FIXME: should loop for more signals if appropriate
	    ;
sigret:
	    pla		; Unstack the syscall return pieces
	    tax
	    pla
	    tay
	    pla
	    plp		; from original stack frame
	    rts

syscout:
;	We may be in decimal mode beyond this line.. take care
;
	    plp

;	Copy the return data over
;
	    ldy _udata + U_DATA__U_RETVAL
	    ldx _udata + U_DATA__U_RETVAL+1
;	Also sets Z for us
	    lda _udata + U_DATA__U_ERROR

	    rts

platform_doexec:
;
;	Start address of executable
;
	    stx ptr1+1
	    sta ptr1

	    clc
	    adc #$20
	    bcc noincx
	    inx
noincx:
	    stx ptr2+1		; Point ptr2 at base + 0x20
	    sta ptr2
	    ldy #0
	    lda (ptr2),y	; Get the signal vector pointer
	    sta PROGLOAD+$20	; if we loaded high put the vecotr in
	    iny
	    lda (ptr2),y
	    sta PROGLOAD+$21	; the low space where it is expected

;
;	Set up the C stack. FIXME: assumes for now our sp in ZP matches it
;
	    lda _udata + U_DATA__U_ISP
	    sta sp
	    ldx _udata + U_DATA__U_ISP+1
            stx sp+1

;
;	Set up the 6502 stack
;
	    ldx #$ff
	    txs
	    ldx #>PROGLOAD	; For the relocation engine
	    lda #ZPBASE
	    ldy #0
	    jmp (ptr1)		; Enter user application

;
;	Straight jumps no funny banking issues
;
_unix_syscall_i:
	    jmp _unix_syscall
_platform_interrupt_i:
	    jmp _platform_interrupt


;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	AX = ptr, length always 512, src and page in globals
;
;	Uses ptr3/4 as 1/2 are reserved for the mappers
;

	.export _hd_read_data,_hd_write_data,_hd_map

_hd_read_data:
	sta ptr3
	stx ptr3+1		; Save the target

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	;
	lda _hd_map
	beq hd_kmap
	jsr map_process_always
hd_kmap:
	ldy #0
	jsr hd_read256
	inc ptr3+1
	jsr hd_read256
	jsr map_kernel
	rts

hd_read256:
	lda $FE34
	sta (ptr3),y
	iny
	bne hd_read256
	rts

_hd_write_data:
	sta ptr3
	stx ptr3+1		; Save the target

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	;
	lda _hd_map
	beq hd_kmapw
	jsr map_process_always
hd_kmapw:
	ldy #0
	jsr hd_write256
	inc ptr3+1
	jsr hd_write256
	jsr map_kernel
	rts

hd_write256:
	lda (ptr3),y
	sta $FE34
	iny
	bne hd_write256
	rts

	.segment "COMMONDATA"

_hd_map:
	.res 1
