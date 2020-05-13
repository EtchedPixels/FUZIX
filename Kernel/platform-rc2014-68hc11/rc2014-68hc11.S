;
;	    rc2014 68HC11 platform functions
;
;	The 68HC11 is a very clean processor so there isn't a lot required here
;

            .globl init_early
            .globl init_hardware
            .globl program_vectors
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_process
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_process_x

            ; exported debugging tools
            .globl platform_monitor
	    .globl platform_reboot
            .globl outchar
	    .globl __hard_di
	    .globl __hard_ei
	    .globl __hard_irqrestore


#include "kernel.def"
#include "../kernel-6303.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .section .common

platform_monitor:
platform_reboot:
	    jsr map_kernel
	    jmp rebootit

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;
;	Disable interrupts and return the old interrupt flag.
;
__hard_di:
	    tpa
	    tab
	    sei			; Save old state in return to C
	    rts
;
;	Enable interrupts
;
__hard_ei:
	    cli			; on 680x cli enables IRQs!!!
	    rts

__hard_irqrestore:
	    tsx
	    ldab 2,x
	    andb #16		; IRQ flag
	    bne irq_on
	    cli
	    rts
irq_on:
	    sei
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .section .text

rebootit:
	    clr 0xFE7B		; top 16K to ROM 0
	    ldx 0xFFFE		; restart vector
	    jmp ,x		; and go

init_early:
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size
	    ldd #512
	    std ramsize
	    ldd #448
	    std procmem
            jmp program_vectors_k

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .section .common

program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    ;
	    ; our C caller will invoke us with the pointer in D
	    ; just pass it on

	    jsr map_process
	    ;
	    ; We need to split these up
	    ;

program_vectors_k:
	    ldd #system_fail
	    std 0xFFFC		; COP fail
	    std 0xFFFA		; watchdog timeout
	    ldd #trap_handler
	    std 0xFFF8		; illegal instruction trap
	    ldd #unix_syscall_entry
	    std 0xFFF6		; SWI vector
	    ldd #nmi_handler
	    std 0xFFF4		; NMI vector
	    ldd #interrupt_handler
	    std 0xFFF2		; Interrupt from RC2014
	    std 0xFFF0		; Interrupt from RTI (RTI overflow)
	    std 0xFFD6		; Interrupt from SC serial
	    ldd #nullirq
	    std 0xFFEE		; TC1
	    std 0xFFEA		; TC2
	    std 0xFFE8		; TOC1
	    std 0xFFE6		; TOC2
	    std 0xFFE4		; TOC3
	    std 0xFFE2		; TOC4
	    std 0xFFE0		; TIC4/TOC5
	    std 0xFFDE		; Timer overflow
	    std 0xFFDC		; Pulse accumulator overflow
	    std 0xFFDA		; Pulse accumulator input edge
	    std 0xFFD8		; SPI transfer complete
	    jmp map_kernel

system_fail:			; we won't see this as the reset will
				; page us back to ROM. Our ROM ought to
				; report it but right now just dies TODO
nullirq:
	    rti

;
;	Map the current process, do not change register values
;
map_process_always:
map_process_always_di:
	    pshx
	    ldx #udata + U_DATA__U_PAGE
	    jsr map_bank
	    pulx
	    rts
;
;	X points to the map table of this process, preserve D
;
map_process:
	    cpx #0
	    bne map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	    pshx
				; Common is left untouched
	    ldx #kmap		; Kernel RAM
	    jsr map_bank
	    pulx
	    rts
kmap:
	    .byte 0x20
	    .byte 0x21
	    .byte 0x22

;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
;	Probably we should use the 8K/56K MMU card.
;
map_process_x:
	    ldab #P_TAB__P_PAGE_OFFSET
	    abx
map_process_2:
map_bank:
	    pshb
	    ; We must load these both before the map changes because we
	    ; might be unmapping the table X points to
	    ldab 2,x
	    ldx ,x
	    stab cur_map+2
	    stab 0xFE7A
	    stx cur_map
	    stx 0xFE78
	    pulb
	    
	    rts



;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	    pshx
	    ldx #saved_map
	    jsr map_bank
	    pulx
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    pshb
	    psha
	    ldab cur_map+2
	    stab saved_map+2	; always save the map in the right commonmem
	    ldd cur_map
	    std saved_map
	    jsr map_kernel
	    pula
	    pulb
	    rts

cur_map:    .byte 0
	    .byte 0
	    .byte 0
saved_map:  .byte 0
	    .byte 0
	    .byte 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    psha
outcharw:
	    ldaa 0xF02E
	    anda #0x20
	    beq outcharw
	    pula
	    stab 0xF02F
	    rts

;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	TOS = ptr, length always 512, src and page in globals
;

	.globl hd_read_data
	.globl hd_write_data
	.globl hd_map

hd_read_data:
	xgdx

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_process
	ldab hd_map
	beq hd_kmap
	jsr map_process_always
hd_kmap:
	clrb
	jsr hd_read256
	jsr hd_read256
	jsr map_kernel
	rts

hd_read256:
	ldaa 0xFE10
	staa ,x
	inx
	incb
	bne hd_read256
	rts

hd_write_data:
	xgdx
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_process
	;
	ldab hd_map
	beq hd_kmapw
	jsr map_process_always
hd_kmapw:
	clrb
	jsr hd_write256
	jsr hd_write256
	jsr map_kernel
	rts

hd_write256:
	ldaa ,x
	staa 0xFE10
	inx
	incb
	bne hd_write256
	rts

hd_map:
	.byte 0
