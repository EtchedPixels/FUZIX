;
;	    rc2014 6502 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
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
#include "zeropage-6303.inc"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

_platform_monitor:
_platform_reboot:
	    clr 0xFE7B		; top 16K to ROM 0
	    jmp ($FFFC)		; check vector FIXME

___hard_di:
	    tpa
	    sei			; Save old state in return to C
	    rts
___hard_ei:
	    cli			; on 6502 cli enables IRQs!!!
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
	    ; Hack for now - create a common copy for init. We should then
	    ; recycle page 32 into a final process but that means awkward
	    ; handling - or does it - we wrap the bit ?? FIXME
	    jsr _create_init_common
	    ldab #36
	    stab 0xFE78		; set low page to copy
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size for test purposes
	    ldd #512
	    std _ramsize
	    ldd #448
	    sta _procmem
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
	    ldd #_interrupt_handler
	    std 0xFFFE		; FIXME: correct vectors
	    ldd #nmi_handler
	    std 0xFFFA		; Ditto
	    ldd #syscall_entry
	    std 0xFFFC		; Ditto
	    jmp map_kernel

map_process_always:
	    pshb
	    ldab _udata + U_DATA__U_PAGE
	    jsr map_bank_i
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
map_kernel:
	    pshb
				; Common is left untouched as is ZP and S
	    ldab #$20		; Kernel RAM
	    jsr map_bank_i
	    pulb
	    rts

;
;	Entry point to map a linear bank range. We switch 4000-BFFF
;	C000-FFFF are constant, 0000-3FFF are switched on the task switch
;	so are valid for map_process_always cases but not mapping an
;	arbitrary process. This is ok - when we add swap it uses
;	map_for_swap and that will map a 16K window in and out (which will
;	need us to fix save/restore)
;
;	FIXME: correct for actual 6303 memory map we use and for 16K x 4
;	bank logic ?
;
map_bank:
	    stab 0xFE78
map_bank_i:			; We are not mapping the first user page yet
	    stab cur_map
	    addb #1
	    stab 0xFE79
	    addb #1
	    stab 0xFE7A
	    rts

; X holds the map table of this process
map_process_2:
	    ldab 0,x
	    jmp map_bank_i


;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	    pshb
	    ldab saved_map	; First bank we skip half of
	    jsr map_bank_i
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
	    pha
outcharw:
	    lda $FEC5
	    and #$20
	    beq outcharw
	    pla
	    sta $FEC0
	    rts

;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	AX = ptr, length always 512, src and page in globals
;

	.export _hd_read_data,_hd_write_data,_hd_map

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

;FIXME linker	.segment "COMMONDATA"

_hd_map:
	.byte 0
