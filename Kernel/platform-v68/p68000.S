/*
 *	Lots left to fill in
 */

		.globl trap_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl trap_monitor
		.globl uarea_block

.mri 1
trap_reboot:
trap_monitor:
	    or #0700,sr
	    bra trap_monitor

init_early:
	    lea.l uarea_block,a5	; udata ptr
            rts

init_hardware:
            ; set system RAM size(hadcode hacks for now) 
	    move.w #512,d0
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now
            rts

;
;	Nothing to do in 68000 - all set up once at boot
;
program_vectors:
	    rts

;
;	TODO
;
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	    rts

; outchar: Wait for UART TX idle, then print the char in d0

outchar:
	    move.w d0,-(sp)
outcharw:
	    move.b $00F03010,d0
	    btst #1,d0
	    beq outcharw
	    move.w (sp)+,d0
	    move.b d0,$00F03000
	    rts

.section data

kernel_flag: byte 1
