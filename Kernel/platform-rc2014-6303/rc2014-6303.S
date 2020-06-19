;
;	    rc2014 6303 platform functions
;
;	The 6303 is a very clean processor so there isn't a lot required here
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_kernel_di
	    .export map_process
	    .export map_process_always
	    .export map_save_kernel
	    .export map_restore
	    .export map_process_x

            ; exported debugging tools
            .export _platform_monitor
	    .export _platform_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

	    .setcpu 6303

#include "kernel.def"
#include "../kernel-6303.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .common

_platform_monitor:
	    ldx #ccreg
	    jsr outstring
	    tsx
	    ldab 2,x
	    jsr outcharhex
	    ldx #dreg
	    jsr outstring
	    tsx
            ldab 4,x
	    jsr outcharhex
	    tsx
            ldab 3,x
	    jsr outcharhex
	    ldx #xreg
            jsr outstring
	    tsx
            ldab 5,x
            jsr outcharhex
	     tsx
            ldab 6,x
            jsr outcharhex
            ldx #pcreg
            jsr outstring
	    tsx
            ldab 7,x
            jsr outcharhex
	    tsx
            ldab 8,x
            jsr outcharhex
            ldx #endtrap
            jsr outstring
	    tsx
	    ldx 10,x
            jsr outx
            ldab #32
	    jsr outchar
	    tsx
	    ldx 12,x
            jsr outx
            ldab #32
	    jsr outchar
	    tsx
	    ldx 14,x
            jsr outx
            ldab #32
	    jsr outchar
	    tsx
	    ldx 16,x
            jsr outx
            ldab #32
	    jsr outchar
	    tsx
	    ldx 18,x
            jsr outx
            ldab #32
	    jsr outchar
	    tsx
	    ldx 20,x
            jsr outx
            ldx #endtrap
	    jsr outstring
_platform_reboot:
	    jsr map_kernel
	    jmp rebootit

ccreg:	    .byte 13,10
	    .ascii "Trap: CC = $"
	    .byte 0
dreg:       .byte 13,10
	    .ascii "      AB = $"
	    .byte 0
xreg:       .byte 13,10
	    .ascii "       X = $"
	    .byte 0
pcreg:      .byte 13,10
	    .ascii "      PC = $"
	    .byte 0
endtrap:    .byte 13,10,13,10,0

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;
;	Disable interrupts and return the old interrupt flag.
;
___hard_di:
	    tpa
	    tab
	    sei			; Save old state in return to C
	    rts
;
;	Enable interrupts
;
___hard_ei:
	    cli			; on 680x cli enables IRQs!!!
	    rts

___hard_irqrestore:
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
            .code

rebootit:
	    clr $FE7B		; top 16K to ROM 0
	    ldx $FFFE		; restart vector
	    jmp ,x		; and go

init_early:
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size
	    ldd #512
	    std _ramsize
	    ldd #448
	    std _procmem
            jmp program_vectors_k

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .common

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    ;
	    ; our C caller will invoke us with the pointer in 3,S
	    ; just pass it on
	    ; our C caller will invoke us with the pointer C style
	    tsx
	    ldx 2,x
	    jsr map_process
	    ;
	    ; We need to split these up
	    ;
program_vectors_k:
	    ldd #interrupt_handler
	    std 0xFFF2		; timer overflow
	    std 0xFFF0		; serial interrupt
	    std 0xFFF8		; external int
	    ldd #nullirq
	    std 0xFFF4		; OCF
	    std 0xFFF6		; ICF
	    ldd #nmi_handler
	    std 0xFFFC		; NMI vector
	    ldd #unix_syscall_entry
	    std 0xFFFA		; SWI vector
	    ldd #trap_handler
	    std 0xFFEE		; 6303 TRAP (usually executing 0 but also
				; illegals)
	    jmp map_kernel

nullirq:
	    rti

;
;	Map the current process, do not change register values
;
map_process_always:
	    pshx
	    ldx #_udata + U_DATA__U_PAGE
	    jsr map_bank
	    pulx
	    rts
;
;	X points to the map table of this process, preserve D
;
map_process:
	    cpx @zero
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
	    ldaa $11
	    anda #$20
	    beq outcharw
	    pula
	    stab $13
	    rts

;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	TOS = ptr, length always 512, src and page in globals
;

	.export _hd_read_data
	.export _hd_write_data
	.export _hd_map

_hd_read_data:
	tsx
	ldx 2,x

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_process
	ldab _hd_map
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

_hd_write_data:
	tsx
	ldx 2,x
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_process
	;
	ldab _hd_map
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

_hd_map:
	.byte 0
