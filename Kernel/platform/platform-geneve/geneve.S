;
;	    Geneve TMS995 platform functions
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
	    .export map_kernel_restore
	    .export map_proc_x

            ; exported debugging tools
            .export _plt_monitor
	    .export _plt_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore

#include "kernel.def"
#include "../../cpu-tms9995/kernel-tms9995.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .common

_plt_monitor:
_plt_reboot:
	jmp	 @_plt_reboot

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;	TODO
;
;	Disable interrupts and return the old interrupt flag.
;
___hard_di:
	stst	r1
	limi	IRQ_OFF
	rt
;
;	Enable interrupts
;
___hard_ei:
	limi	IRQ_ON
	rt

___hard_irqrestore:
	lst	r4
	rt

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
	rt			; stack was copied so this is ok

init_hardware:
	dect	r13
	mov	r11,*r13
	; TODO
        ; set system RAM size
	li	r0,512
	mov	r0,@_ramsize
	li	r0,448
	mov	r0,@_procmem
        bl	@program_vectors_k
	bl	@_vdpinit
	mov	*r13+,r11
	rt

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .common

_program_vectors:
        ; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	;
	; our C caller will invoke us with the pointer C style in r4

	dect	r13
	mov	r11,*r13
	mov	r4,r0
	bl	@map_proc
	bl	@program_vectors_k
	bl	@map_kernel
	mov	*r13+,r11
	rt

	;
	; TODO
	;
program_vectors_k:
	li	r0, vectab
	; Copy interrupt vectors
	mov	*r0+,*r1+
	mov	*r0+,*r1+
	mov	*r0+,*r1+
	mov	*r0+,*r1+
	mov	*r0+,*r1+
	mov	*r0,*r1
	li	r0, 0x0040
	li	r1, xop_ws
	li	r2, xop_trap
	li	r3, 16
xopl:
	mov	r1, *r0+
	mov	r2, *r0+
	dec	r3
	jne	@xopl
	rt
vectab:
	.word	irq1_ws			; Interrupt
	.word	irq1_handler
	.word	irq2_ws			; MID, overflow and decrementer
	.word	irq23_handler
	.word	irq4_ws			; Interrupt on extension pin
	.word	irq4_handler

;
;	Actual handlers
;
irq1_handler:
irq4_handler:
overflow_interrupt:
	rtwp
irq23_handler:
	li	r12, 0x1fda
	tb	0
	jne	@mid_interrupt		; illegal
	li	r12, 0x1ee6
	tb	0
	jne	@overflow_interrupt
	b	@interrupt_handler
mid_interrupt:
xop_trap:
	b	@trap_handler


;
;	Maop routines - must save any register changed except r0
;

;
;	Map the current process.
;
map_proc_always:
map_proc_always_di:
	mov	@_udata + U_DATA__U_PAGE,r0
	jmp	@map_bank
;
;	Map process in r0, use r0
;
map_proc:
	ci	r0,0
	jne	@map_proc_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	li	r0,kmap
	jmp	@map_bank
kmap:
	.byte	0x38
	.byte	0x39
	.byte	0x3A
	.byte	0x3B
	.byte	0x3C
	.byte	0x3D
	.byte	0x3E
	.byte	0x3F
	.even
;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
;	Probably we should use the 8K/56K MMU card.
;
map_proc_x:
	; r0 holds the process
	ai	r0, P_TAB__P_PAGE_OFFSET
	mov	*r0,r0
map_proc_2:
map_bank:
	mov	*r0, @cur_map
	mov	*r0+,@0xF110
	mov	*r0, @cur_map+2
	mov	*r0+,@0xF112
	mov	*r0, @cur_map+4
	mov	*r0+,@0xF114
	mov	*r0, @cur_map+6
	mov	*r0, @0xF116
	rt

;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_kernel_restore:
	li	r0,saved_map
	jmp	@map_bank

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	mov	@cur_map, @saved_map
	mov	@cur_map+2, @saved_map+2
	mov	@cur_map+4, @saved_map+4
	mov	@cur_map+6, @saved_map+6
	li	r0,kmap
	jmp	@map_bank

cur_map:
	.word 0
	.word 0
	.word 0
	.word 0

saved_map:
	.word 0
	.word 0
	.word 0
	.word 0

;
;	Tricky as we have no obvious debug I/O. Cross our fingers that
;	the kernel vdp is intact
;
outchar:
	dect	r13
	mov	r1,*r13
	dect	r13
	mov	r2,*r13
	dect	r13
	mov	r3,*r13
	dect	r13
	mov	r4,*r13
	dect	r13
	mov	r5,*r13
	mov	r0,r4
	bl	@_kputchar
	mov	*r13+,r5
	mov	*r13+,r4
	mov	*r13+,r3
	mov	*r13+,r2
	mov	*r13+,r1
	rt
lit_2000:
	.word	0x2000

;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	TOS = ptr, length always 512, src and page in globals
;
;	FIXME: swap support
;
;	FIXME: optimize word aligned case with movb/swpb/movb/store type
; stuff
;

	.export _hd_read_data
	.export _hd_write_data
	.export _hd_map

; Enters with R4 = buffer pointer
_hd_read_data:
	mov	r11, r3
	li	r2,512
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_proc
	movb	@_hd_map,r0
	jeq	@hd_kmap
	bl	@map_proc_always
hd_kmap:
	movb	@0xFE10,*r4+
	dec	r2
	jne	@hd_kmap
	bl	@map_kernel
	b	*r3

_hd_write_data:
	mov	r11,r3
	li	r2,512
	movb	@_hd_map,r0
	jeq	@hd_kmapw
	bl	@map_proc_always
hd_kmapw:
	movb	*r4+,@0xFE10
	dec	r2
	jne	@hd_kmapw
	bl	@map_kernel
	b	*r3

_hd_map:
	.byte 0

	.even

		.dp		; 16bit fast memory at 0xF000

		.export kernel_ws
		.export irq1_ws
		.export irq2_ws
		.export irq4_ws
		.export xop_ws
		.export nmi_ws
;
;	Workspaces
;
kernel_ws:
	.ds	32
irq1_ws:
	.ds	32
irq2_ws:
	.ds	32
irq4_ws:
	.ds	32
xop_ws:
	.ds	32
nmi_ws:
	.ds	32
