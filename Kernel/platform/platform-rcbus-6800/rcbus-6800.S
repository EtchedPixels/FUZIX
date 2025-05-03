;
;	    RCBUS 6800 platform functions
;
;	The 6800 is a very clean processor so there isn't a lot required
;	here, it's a bit longer winded compared with 6803/6303
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_kernel_di
	    .export map_proc
	    .export map_proc_always
	    .export map_proc_always_di
	    .export map_save_kernel
	    .export map_restore
	    .export map_proc_x

            ; exported debugging tools
            .export _plt_monitor
	    .export _plt_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

#include "kernel.def"
#include "../../cpu-6800/kernel-6800.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .common

_plt_monitor:
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

	    ldaa #8
stkrow:
	    ldab #13
	    jsr outchar
	    ldab #10
	    jsr outchar
	    jsr outx
            ldab #':'
	    jsr outchar
	    ldab #' '
            jsr outchar
            clrb
stkword:
	    pshb
	    psha
	    ldaa 10,x
	    ldab 11,x
            jsr outd
	    pula
	    pulb
	    inx
	    inx
	    incb
	    cmpb #8
	    beq endrow
	    pshb
            ldab #32
	    jsr outchar
	    pulb
	    bra stkword
endrow:	    deca
	    bne stkrow
	    ldab #13
	    jsr outchar
	    ldab #10
	    jsr outchar

_plt_reboot:
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
endtrap:    .byte 13,10,0

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
	    ldaa 2,x
	    tap
	    jmp __cleanup1

irq_on:
	    sei
	    jmp __cleanup1

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
	    ldx #$FE60		; PTM
	    lda #0x01
	    sta 1,x		; CR1, CR2, square no IRQ refclk, 16bit
	    sta 0,x		; Reset
	    ldaa #0x59		; 23039 + 1 * 8 -> 184320 for 10Hz
	    ldab #0xFC
	    staa 6,x		; Set up the timer
	    stab 7,x
	    clr  1,x		; CR2 square, no IRQW, no out, no in, CR3 on
	    ldaa #0x43
	    staa ,x		; counter, count E clocks, prescale, IRQ
	    ldaa #0x01
	    staa 1,x		; back to CR1
	    clr ,x		; out of reset

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
	    jsr map_proc
	    jsr program_vectors_k
	    jmp __cleanup2

	    ;
	    ; We need to split these up
	    ;
program_vectors_k:
	    ldx #0xFF00
	    ldaa #>interrupt_handler
	    ldab #<interrupt_handler
	    staa 0xF8,x		; timer overflow
	    stab 0xF9,x
	    ldaa #>unix_syscall_entry
	    ldab #<unix_syscall_entry
	    staa 0xFA,x		; SWI vector
	    stab 0xFB,x
	    ldaa #>nmi_handler
	    ldab #<nmi_handler
	    staa 0xFC,x		; NMI vector
	    stab 0xFD,x
	    jmp map_kernel

;
;	Map the current process, do not change register values
;
map_proc_always:
map_proc_always_di:
	    psha
	    tpa
	    psha
	    sei
	    stx mapsavex
	    ldx #_udata + U_DATA__U_PAGE
	    jsr map_bank
	    ldx mapsavex
	    pula
	    tap
	    pula
	    rts
;
;	X points to the map table of this process, preserve D
;
map_proc:
	    cpx @zero
	    bne map_proc_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	    psha
	    tpa
	    psha
	    sei
	    stx mapsavex
				; Common is left untouched
	    ldx #kmap		; Kernel RAM
	    jsr map_bank
	    ldx mapsavex
	    pula
	    tap
	    pula
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
map_proc_x:
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


map_proc_2:
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
	    psha
	    tpa
	    psha
	    sei
	    stx mapsavex
	    ldx #saved_map
	    jsr map_bank
	    ldx mapsavex
	    pula
	    tap
	    pula
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
	    ldaa 0xFEC5
	    anda #0x20
	    beq outcharw
	    pula
	    stab 0xFEC0
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
	; and we must reference them before we map_proc
	ldab _hd_map
	beq hd_kmap
	jsr map_proc_always
hd_kmap:
	clrb
	bsr hd_read256
	bsr hd_read256
	jsr map_kernel
	jmp __cleanup2

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
	; and we must reference them before we map_proc
	;
	ldab _hd_map
	beq hd_kmapw
	jsr map_proc_always
hd_kmapw:
	clrb
	bsr hd_write256
	bsr hd_write256
	jsr map_kernel
	jmp __cleanup2

hd_write256:
	ldaa ,x
	staa 0xFE10
	inx
	incb
	bne hd_write256
	rts

_hd_map:
	.byte 0
