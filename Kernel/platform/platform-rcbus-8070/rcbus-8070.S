;
;	RCBUS 8070 support.
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
	.export map_proc_p2

        ; exported debugging tools
        .export _plt_monitor
	.export _plt_reboot
        .export outchar
	.export ___hard_di
	.export ___hard_ei
	.export ___hard_irqrestore

#include "kernel.def"
#include "../../cpu-8070/kernel-8070.def"

;
;	Direct page stuff
;
	.dp

cur_map:		; Memory bank reg copies
	.byte 0
	.byte 0
	.byte 0

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	.common

_plt_monitor:
_plt_reboot:
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
	ld	a,s
	and	s,=0xFE
	ret
;
;	Enable interrupts
;
___hard_ei:
	or	a,=1
	ret

___hard_irqrestore:
	ld	a,2,p1
	ld	s,a
	ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .code

rebootit:
	ld	a,=0
	ld	p2,=0xFE78
	st	a,0,p2		; ROM low
	jmp	0x0000

init_early:
        ret			; stack was copied so this is ok

init_hardware:
        ; set system RAM size
	ld	ea,=512
	ld	p2,=_ramsize
	st	ea,0,p2
	ld	ea,=448
	ld	p2,=_procmem
	st	ea,0,p2
        jmp	program_vectors_k

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

       .common

; We don't have vectors as such. There's fixed functions in the low
; space that we have to honour (1 reset, 4 int A, 7 int B)
_program_vectors:
        ; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	;
	; our C caller will invoke us with the pointer C style
	ld	ea,2,p1
	jsr	map_proc
program_vectors_k:
	ld	p2,=0
	ld	p3,=lowbuf
	ld	ea,@2,p3
	st	ea,@2,p2
	ld	ea,@2,p3
	st	ea,@2,p2
	ld	ea,@2,p3
	st	ea,@2,p2
	ld	ea,@2,p3
	st	ea,@2,p2
	ld	ea,@2,p3
	st	ea,@2,p2
	jmp	map_kernel

lowbuf:
	nop
	jmp	trap_handler
	jmp	interrupt_handler
	jmp	nmi_handler

;
;	Map the current process, do not change register values
;
map_proc_always:
map_proc_always_di:
	push	p2
	ld	p2,=_udata + U_DATA__U_PAGE
	jsr	map_proc_p2
	pop	p2
	ret
;
;	X points to the map table of this process, preserve D
;
map_proc:
	push	ea
	or	a,e
	bnz	map_proc_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	push	p2
	ld	p2,=kmap		; Kernel RAM
	jsr	map_proc_p2
	pop	p2
	ret
kmap:
	.byte 0x20
	.byte 0x21
	.byte 0x22

; Map the page we are swapping at 0x8000
map_for_swap:
	push	p2
	ld	p2,=0xFE78
	st	a,:cur_map+2
	st	a,2,p2
	pop	p2
	ret
;
;	For now we are using the 4x16K MMU but this will probably change.
;	Switch the low 48K, the task switch flips the top. Hack until we
;	enable the real MMU and banking setup.
;
;	Probably we should use the 8K/56K MMU card.
;
map_proc_2:
	push	p2
	ld	ea,=P_TAB__P_PAGE_OFFSET
	add	ea,2,p1		; add the passed EA (process ptr)
	ld	p2,ea
	jsr	map_proc_p2
	pop	p2
	pop	ea
	ret
	;	Care needed as we unmap the data we are pointing into
	;	so must read it all via T first
map_proc_p2:
	push	ea
map_bank:
	push	p3
	ld	p3,=0xFE78		; MMU
	ld	ea,0,p2
	ld	t,ea
	ld	a,2,p2
	st	a,:cur_map+2
	ld	ea,t
	st	ea,:cur_map
	st	ea,0,p3
	pop	p3
	pop	ea
	ret

;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	push	p2
	ld	p2,=saved_map
	jsr	map_bank
	pop	p2
	ret

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	push	ea
	push	p2
	ld	p2,=saved_map
	ld	ea,:cur_map
	st	ea,0,p2
	ld	a,:cur_map+2
	st	a,2,p2
	ld	p2,=kmap	
	jsr	map_bank
	pop	p2
	pop	ea
	ret

	
saved_map:
	.byte	0
	.byte	0
	.byte	0


; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	push	p3
	push	ea
	ld	p3,=0xFEC0
	ld	e,a
outcharw:
	ld	a,5,p3
	and	a,=0x20
	bz	outcharw
	ld	a,e
	st	a,0,p3
	pop	ea
	pop	p3
justret:
	ret
;
;	Disk copier (needs to be in common)
;

	.export _devide_read_data
	.export _devide_write_data

map_and_setup:
	ld	a,=0
	st	a,:__tmp		; counter
	ld	ea,4,p1			; data pointer
	ld	p3,ea
	ld	p2,=_td_raw
	ld	a,0,p2
	bz	justret			; kernel
	sub	a,=2
	bz	is_swap
	jmp	map_proc_always		; user
is_swap:
	ld	p2,=_td_page
	ld	a,0,p2
	jmp	map_for_swap		; map that page


_devide_read_data:
	jsr	map_and_setup
	ld	p2,=0xFE10
hd_read:
	ld	a,0,p2
	st	a,@1,p3
	ld	a,0,p2
	st	a,@1,p3
	dld	a,:__tmp
	bnz	hd_read
	jmp	map_kernel

_devide_write_data:
	jsr	map_and_setup
	ld	p2,=0xFE10
hd_write:
	ld	a,@1,p3
	st	a,0,p2
	ld	a,@1,p3
	st	a,0,p2
	dld	a,:__tmp
	bnz	hd_write
	jmp	map_kernel
