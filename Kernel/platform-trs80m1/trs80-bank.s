;
;	    TRS 80 banking logic for the base Model 4 and 4P
;

            .module trs80bank

            ; exported symbols
            .globl init_hardware
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_save
	    .globl map_kernel_restore
	    .globl map_save
	    .globl map_restore

            ; imported symbols
	    .globl _program_vectors	
            .globl _ramsize
            .globl _procmem

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 08000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _BOOT

init_hardware:
            ; set system RAM size
	    ld hl,#0xFFFF		; FFFF is free in all our pages
	    ld bc,#0xFF43		; kernel included
mark_pages:
	    out (c),b
	    ld (hl),b
	    djnz mark_pages
scan_pages:
	    ld a,#2
scan_pages_l:
	    out (c),a
	    cp (hl)
	    jr nz, mismatch
	    inc a
	    jr nz, scan_pages_l
mismatch:				; A holds the first page above
					; the limit (4 for 128K etc)
	    dec a
	    dec a			; remove kernel
	    ld h,a
	    ld l,#0			; effectively * 256
	    srl h
	    rr  l			; * 128
	    srl h
	    rr	l			; * 64
	    srl h
	    rr  l			; * 32 so now in Kbytes
            ld (_procmem), hl
	    ld de,#80			; Kernel costs us 80K due to bank
	    add hl,de			; annoyances
            ld (_ramsize), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

ksave_map:  .db 0x00	; saved kernel map version
map_port:   .db 0x43	; cheap hack so we can ld bc and out (c),b
map_reg:    .db 0x00	; current value written to map register
map_store:  .db 0x00	; save value from map_save
;
;	Map in the kernel. We are in common and all kernel entry points are
;	in common. Thus someone will always have banked in the right page
;	before jumping out of common. Thus we don't need to do anything but
;	restore the last saved kernel map!
;
map_kernel:
map_kernel_restore:
	    push af
	    ld a, (ksave_map)
	    ld (map_reg), a
	    out (0x43), a
	    pop af
	    ret
;
;	Select the bank for the relevant process. Update the ksave_map so we
;	can restore the correct kernel mapping when banked.
;
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
            ld a, (map_reg)
	    ld (ksave_map),a	; for map_kernel_restore
	    ld a,(hl)		; udata page
	    ld (map_reg),a
	    out (0x43), a
            ret

map_process_a:			; used by bankfork
	    push af
	    ld a, (map_reg)
	    ld (ksave_map), a
	    pop af
	    ld (map_reg), a
	    out (0x43), a
	    ret

map_process_save:
map_process_always:
	    push af
	    ld a, (map_reg);
	    ld (map_store), a
	    pop af
	    ret

map_save:   push af
	    ld a, (map_reg)
	    ld (map_store), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (map_store)
	    ld (map_reg), a
	    out (0x43), a
	    pop af
	    ret

;
;	This lot is tricky.
;
	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_1_2
	.globl __bank_2_1
	.globl __stub_0_1
	.globl __stub_0_2

;
;	Start with the easy ones - we are going from common
;	to a bank. We can use registers here providing the compiler expects
;	them to be destroyed by the called function, and on the return
;	providing they are also not a return value. In practice that means
;	we need to avoid IX going in and IX HL DE coming out.
;
;	The linker rewrote
;			push af call foo pop af
;	into
;			call __bank_0_1 defw foo
;
;	We expand these into separate functions as they are executed a fair
;	bit
;
;	The only hard case to understand logically here is calls and stubs
;	from common to a bank. In those cases we must save the previous bank
;	and restore it. We do this because we optimise the performance by
;	making calls into COMMON or CODE free of bank work so we can put
;	performance sensitive widely used routines (like compiler helpers)
;	there. The cost of that is that if we do call back out of common
;	space we have to do the restore as we return.
;
__bank_0_2:			; Call from common to bank 2
	ld a,#1
	jr bankina
__bank_0_1:			; Call from common to bank 1
	xor a
bankina:
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld bc,(map_port)
	ld (map_reg),a
	out (c),a
	ex de,hl
	ld a,b
	or a
	jr z, retbank1		; Arrange that we put the banks back as they
	call callhl		; were before the call. Needed because calls
	ld a,#1			; to common routines don't do banking so
	ld (map_reg),a		; the return won't either.
	out (0x43),a
	ret
retbank1:
	call callhl
	xor a
	ld (map_reg),a
	out (0x43),a
	ret

__bank_1_2:
	ld a,#1
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld bc,(map_port)
	ld (map_reg),a
	out (c),a
	ex de,hl
	call callhl
	xor a
	ld (map_reg),a
	out (0x43),a
	ret

__bank_2_1:
	xor a
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld bc,(map_port)
	ld (map_reg),a
	out (c),a
	ex de,hl
	call callhl
	ld a,#1
	ld (map_reg),a
	out (0x43),a
	ret

;
;	Stubs are similar. Where there is a function pointer the linker adds
;	a call in common and resolves the function pointer to the code in
;	common. The function stub it adds is simply
;
;		ld de,#realaddr
;		call __stub_0_%d   where %d is the target bank
;
;	For the same reason as bank calls we need to restore the previous
;	banking setup.
;
__stub_0_1:
	xor a
	jr stub_call
__stub_0_2:
	ld a,#1
stub_call:
	pop hl			; return address
	ex (sp),hl		; swap the junk word for the return
	ld bc,(map_port)
	ld (map_reg),a
	out (c),a
	ex de,hl
	ld a,b
	or a
	jr z, stub_ret_1
	call callhl
	ld a,#1
	jr stub_ret
stub_ret_1:
	call callhl		; invoke code in other bank
	xor a			; and back to bank 1
stub_ret:
	ld (map_reg),a
	out (0x43),a
	pop bc			; bc is now the return
	push bc			; stack it twice
	push bc
	ret			; and ret - can't use jp (ix) or jp (hl) here
callhl:	jp (hl)
