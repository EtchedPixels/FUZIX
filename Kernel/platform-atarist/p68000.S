/*
 *	    68000 Atari ST
 *
 *	We need pretty much nothing here as we don't do page mapping.
 */

		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl platform_reboot,platform_monitor
		.globl scroll_down,scroll_up,plot_char,clear_lines
		.globl clear_across,cursor_on,cursor_off
		.globl cursor_disable, vtattr_notify
		.globl fontdata_8x8, screenbase
		.globl uarea_block

.mri 1
platform_reboot:
platform_monitor:
	    or #0700,sr
	    bra platform_monitor
init_early:
	    lea.l uarea_block,a5	; udata ptr
	    move.l a5,udata_shadow
            rts

init_hardware:
            ; set system RAM size
	    move.l $42E,d0		; TOS phystop
	    lsr.l #8,d0			; into KBytes
	    lsr.l #2,d0
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now
	    bsr install_vectors
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

plot_char:
	    ; y, x, c
	    move.l screenbase,a0	; Screen base from TOS
	    move.w 4(sp),d0
	    mulu.w #640,d0		; Always under 64K
	    adda.l d0,a0
	    adda.w 6(sp),a0		; do is now our offset

	    move.w 8(sp),d0		; character
	    lsl.w #3,d0			; into font position
	    lea fontdata_8x8,a1	; font
	    adda.w d0,a1
	    moveq #7,d1
plotit:
	    move.b (a1)+,(a0)
	    add.l #80,a0
	    dbra d1,plotit
	    rts

scroll_down:
scroll_up:
clear_lines:
clear_across:
cursor_on:
cursor_off:
cursor_disable:
vtattr_notify:
	    rts	

.section data

kernel_flag: byte 1
