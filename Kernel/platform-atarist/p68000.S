/*
 *	    68000 Atari ST
 *
 *	We need pretty much nothing here as we don't do page mapping.
 */

		.globl trap_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl trap_reboot, trap_monitor
.mri 1
trap_reboot:
trap_monitor:
	    or #0700,sr
	    bra trap_monitor
init_early:
            rts

init_hardware:
            ; set system RAM size
	    move.w #512,ramsize
	    move.w #448,procmem	; guesses for now
            rts


;	Nothing to do in 68000 - all set up once at boot
;
program_vectors:
	    rts

;
;	All mapped all the time
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
	    move.b $FFFFFA13,d0
	    btst #2,d0
	    beq outcharw
	    move.w (sp)+,d0
	    move.b d0,$FFFFFA2D
	    rts
	
.section data

_kernel_flag: byte 1
