	.module ti84pse

	; exported symbols
	.globl init_hardware
	.globl _program_vectors
	.globl platform_interrupt_all

	.globl map_kernel
	.globl map_kernel_di
	.globl map_process
	.globl map_process_di
	.globl map_process_always
	.globl map_process_always_di
	.globl map_process_a
	.globl map_save_kernel
	.globl map_restore

	.globl _int_disabled

	.globl _platform_reboot
	.globl _platform_monitor
	.globl outchar

	; imported symbols
	.globl _ramsize
	.globl _procmem
	.globl interrupt_handler

	.include "kernel.def"
	.include "../kernel-z80.def"

	.area _COMMONMEM

_platform_monitor:
	di
	halt ; TODO
platform_interrupt_all:
	ret
	.ascii "platform_monitor"

_platform_reboot:
	di
	halt ; TODO

	.area _CODE

init_hardware:
	; TODO: Double check what units this is in (KiB presumed)
	ld hl, #32
	ld (_ramsize), hl
	ld hl, #32
	ld (_procmem), hl

	; set up interrupt vectors for the kernel
	ld hl, #0
	push hl
	call _program_vectors
	pop hl

	; TODO: Double check interrupt table
	ld a, #0xfe			; Use FEFF (currently free)
	ld i, a
	im 2 ; set CPU interrupt mode
	ret

	.area _COMMONMEM

_int_disabled:
	.byte 1

; XXX: I'm not sure we actually need to repeatedly program these vectors in
; high memory for this platform.
_program_vectors:
	; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	di
	pop de ; temporarily store return address
	pop hl ; function argument -- base page number
	push hl ; put stack back as it was
	push de

	call map_process

	; install the interrupt vector at 0xFEFF
	ld hl, #interrupt_handler
	ld (0xFEFF), hl
	; now install the interrupt vector at 0xFEFF
	ld hl, #interrupt_handler
	ld (0xFEFF), hl

	; Fallthrough

map_kernel:
map_kernel_di:
	push af
	ld a, #1
	out (6), a
	inc a
	out (7), a
	pop af
	ret

map_process:
map_process_di:
	ld a, h
	or l
	jr z, map_kernel
	di
	halt ; TODO
	.ascii "map_process"

map_process_a:
	di
	halt ; TODO
	.ascii "map_process_a"

map_process_always:
map_process_always_di:
	push af
	push hl
	ld hl,#U_DATA__U_PAGE
	call map_process
	pop hl
	pop af
	ret

map_save_kernel:
	di
	halt ; TODO
	.ascii "map_save_kernel"
map_restore:
	di
	halt ; TODO
	.ascii "map_restore"

outchar:
	di
	halt ; TODO
	.ascii "outchar"

mmu_kernel:
	di
	halt ; TODO
	.ascii "mmu_kernel"

mmu_kernel_irq:
	di
	halt ; TODO
	.ascii "mmu_kernel_irq"

mmu_restore_irq:
	di
	halt ; TODO
	.ascii "mmu_restore_irq"
mmu_user:
	di
	halt ; TODO
	.ascii "mmu_user"
