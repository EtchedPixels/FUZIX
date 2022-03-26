;
;	    rc2014 TMS995 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_kernel_di
	    .export map_process
	    .export map_process_always
	    .export map_process_always_di
	    .export map_save_kernel
	    .export map_kernel_restore
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
	    jmp @_platform_reboot

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;	TODO
;
;	Disable interrupts and return the old interrupt flag.
;
___hard_di:
	    rt
;
;	Enable interrupts
;
___hard_ei:
	    rt

___hard_irqrestore:
	    rt

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
            rt			; stack was copied so this is ok

init_hardware:
            ; set system RAM size
	    li r0,512
	    mov r0,@_ramsize
	    li r0,448
	    mov r0,@_procmem
            b @program_vectors_k

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

	    mov *r13,r0
	    dect r13
	    mov r11,*r13
	    bl @map_process
	    bl @program_vectors_k
	    bl @map_kernel
	    mov *r13+,r11
	    rt
	    ;
	    ; TODO
	    ;
program_vectors_k:
	    rt

;
;	Map the current process, do not change register values except r0
;
map_process_always:
map_process_always_di:
	    mov @_udata + U_DATA__U_PAGE,r0
	    mov r11,*r13
	    bl @map_bank
	    mov *r13+,r11
	    rt
;
;	Map process in r0, use r0
;
map_process:
	    ci	r0,0
	    jne @map_process_2
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
	    jmp @map_bank
kmap:
	    .byte 0x20
	    .byte 0x21
	    .byte 0x22
	    .even
;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
;	Probably we should use the 8K/56K MMU card.
;
map_process_x:
	    ; r0 holds the process
	    ai r0,P_TAB__P_PAGE_OFFSET
map_process_2:
map_bank:
	    mov		*r0,@cur_map
	    mov		*r0+,@0xFE78
	    movb	*r0,@cur_map+2
	    movb	*r0+,@0xFE7A
	    rt

;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_kernel_restore:
	    li	r0,saved_map
	    jmp @map_bank

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    mov @cur_map,@saved_map
	    movb @cur_map+2,@saved_map+2
	    li r0,kmap
	    jmp @map_bank

cur_map:    .byte 0
	    .byte 0
	    .byte 0
	    .even
saved_map:  .byte 0
	    .byte 0
	    .byte 0
	    .even

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    rt

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

_hd_read_data:
	mov	*r13,r1
	dect	r13
	mov	r11,*r13
	li	r2,512
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_process
	movb @_hd_map,r0
	jeq @hd_kmap
	bl @map_process_always
hd_kmap:
	movb @0xFE10,*r1+
	dec r2
	jne @hd_kmap
	bl @map_kernel
	mov *r13+,r11
	rt

_hd_write_data:
	mov	*r13,r1
	dect	r13
	mov	r11,*r13
	li	r2,512
	movb @_hd_map,r0
	jeq @hd_kmapw
	bl @map_process_always
hd_kmapw:
	movb *r1+,@0xFE10
	dec r2
	jne @hd_kmapw
	bl @map_kernel
	mov *r13+,r11
	rt

_hd_map:
	.byte 0
