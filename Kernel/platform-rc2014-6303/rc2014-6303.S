;
;	    rc2014 6502 platform functions
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

            ; exported debugging tools
            .export _platform_monitor
	    .export _platform_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

#include "kernel.def"
#include "../kernel-6303.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------

	    .common

_platform_monitor:
_platform_reboot:
	    clr $FE7B		; top 16K to ROM 0
	    ldx $FFFC
	    jmp ,x		; check vector FIXME

___hard_di:
	    tpa
	    tab
	    sei			; Save old state in return to C
	    rts
___hard_ei:
	    cli			; on 680x cli enables IRQs!!!
	    rts

___hard_irqrestore:
	    tsx
	    ldab 3,x
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

init_early:
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size for test purposes
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
	    sei
	    ;
	    ; our C caller will invoke us with the pointer in x,a
	    ; just pass it on
	    jsr map_process
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

map_process_always:
	    pshb
	    ldab _udata + U_DATA__U_PAGE
	    jsr map_bank
	    pulb
	    rts
;
;	X points to the map table of this process
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
	    pshb
				; Common is left untouched
	    ldab #$20		; Kernel RAM
	    jsr map_bank
	    pulb
	    rts

;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
map_bank:
	    stab 0xFE78
	    stab cur_map
	    addb #1
	    stab 0xFE79
	    addb #1
	    stab 0xFE7A
	    ; And don't swap the common here but in the task switch
	    rts

; X holds the map table of this process
map_process_2:
	    ldab 0,x
	    jmp map_bank


;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	    pshb
	    ldab saved_map	; First bank we skip half of
	    jsr map_bank
	    pulb
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    pshb
	    ldab cur_map
	    stab saved_map	; always save the map in the right commonmem
	    jsr map_kernel
	    pulb
	    rts

cur_map:    .byte 0
saved_map:  .byte 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    psha
outcharw:
	    ldaa $11
	    anda #$20
	    beq outcharw
	    pula
	    staa $13
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
	ldx 3,x

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	;
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
	addb #1
	bne hd_read256
	rts

_hd_write_data:
	tsx
	ldx 3,x
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
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
	addb #1
	bne hd_write256
	rts

_hd_map:
	.byte 0
