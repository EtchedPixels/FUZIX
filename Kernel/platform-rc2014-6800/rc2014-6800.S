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

#include "kernel.def"
#include "../kernel-6303.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .common

_platform_monitor:
_platform_reboot:
	    jsr map_kernel
	    jmp rebootit

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
	    ldab 3,x
	    andb #16		; IRQ flag
	    bne irq_on
	    cli
	    jmp ret1

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
	    ldaa #>512
	    ldab #<512
	    staa _ramsize
	    stab _ramsize+1
	    ldaa #>448
	    ldab #<448
	    staa _procmem
	    stab _procmem+1
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
	    jsr program_vectors_k
	    jmp ret2

	    ;
	    ; We need to split these up
	    ;
program_vectors_k:
	    ldx #0xFF00
	    ldaa #>interrupt_handler
	    ldab #<interrupt_handler
	    staa 0xF2,x		; timer overflow
	    stab 0xF3,x
	    staa 0xF0,x		; serial interrupt
	    stab 0xF1,x
	    staa 0xF8,x		; external int
	    stab 0xF9,x
	    ldaa #>nullirq
	    ldab #<nullirq
	    staa 0xF4,x		; OCF
	    stab 0xF5,x
	    staa 0xF6,x		; ICF
	    stab 0xF7,x
	    ldaa #>nmi_handler
	    ldab #<nmi_handler
	    staa 0xFC,x		; NMI vector
	    stab 0xFD,x
	    ldaa #>unix_syscall_entry
	    ldab #<unix_syscall_entry
	    staa 0xFA,x		; SWI vector
	    stab 0xFB,x
	    jmp map_kernel

nullirq:
	    rti

;
;	Map the current process, do not change register values
;
map_process_always:
	    stx mapsavex
	    ldx #_udata + U_DATA__U_PAGE
	    jsr map_bank
	    ldx mapsavex
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
	    stx mapsavex
				; Common is left untouched
	    ldx #kmap		; Kernel RAM
	    jsr map_bank
	    ldx mapsavex
	    rts
kmap:
	    .byte 0x20
	    .byte 0x21
	    .byte 0x22
mapsavex:
	    .word 0x0000	; save X for map changes, must be common

;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
;	Probably we should use the 8K/56K MMU card.
;
map_process_x:
	    ; We must load these both before the map changes because we
	    ; might be unmapping the table X points to
	    pshb
	    ldab P_TAB__P_PAGE_OFFSET+2,x
	    ldx  P_TAB__P_PAGE_OFFSET,x
	    stab cur_map+2
	    stab 0xFE7A
	    stx cur_map
	    stx 0xFE78
	    pulb
	    
	    rts


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
	    stx mapsavex
	    ldx #saved_map
	    jsr map_bank
	    ldx mapsavex
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    psha
	    ldaa cur_map+2
	    staa saved_map+2	; always save the map in the right commonmem
	    ldaa cur_map
	    staa saved_map
	    ldaa cur_map+1
	    staa saved_map+1
	    jsr map_kernel
	    pula
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
	    ldaa 0x11
	    anda #0x20
	    beq outcharw
	    pula
	    stab 0x13
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
	bsr hd_read256
	bsr hd_read256
	jsr map_kernel
	jmp ret2

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
	bsr hd_write256
	bsr hd_write256
	jsr map_kernel
	jmp ret2

hd_write256:
	ldaa ,x
	staa 0xFE10
	inx
	incb
	bne hd_write256
	rts

_hd_map:
	.byte 0
