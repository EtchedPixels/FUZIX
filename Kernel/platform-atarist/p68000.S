/*
 *	    68000 Atari ST
 *
 *	We need pretty much nothing here as we don't do page mapping.
 */

		.globl _trap_reboot
		.globl init_early
		.globl init_hardware
		.globl _program_vectors
		.globl outchar
		.globl _trap_reboot,_trap_monitor
		.globl _scroll_down,_scroll_up,_plot_char,_clear_lines
		.globl _clear_across,_cursor_on,_cursor_off
		.globl _fontdata_8x8, _screenbase
		.globl _uarea_block

.mri 1
_trap_reboot:
_trap_monitor:
	    or #0700,sr
	    bra _trap_monitor
init_early:
	    lea.l _uarea_block,a5	; udata ptr
            rts

init_hardware:
            ; set system RAM size
	    move.l $42E,d0		; TOS phystop
	    lsr.l #8,d0			; into KBytes
	    lsr.l #2,d0
	    move.w d0,_ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,_procmem		; guesses for now
            rts


;	Nothing to do in 68000 - all set up once at boot
;
_program_vectors:
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

_scroll_down:
_scroll_up:
_plot_char:
	    ; y, x, c
	    move.l _screenbase,a0	; Screen base from TOS
	    move.w 4(sp),d0
	    mulu.w #640,d0		; Always under 64K
	    adda.l d0,a0
	    adda.w 6(sp),a0		; do is now our offset

	    move.w 8(sp),d0		; character
	    lsl.w #3,d0			; into font position
	    lea _fontdata_8x8,a1	; font
	    adda.w d0,a1
	    moveq #7,d1
_plotit:
	    move.b (a1)+,(a0)
	    add.l #80,a0
	    dbra d1,_plotit
	    rts

_clear_lines:
_clear_across:
_cursor_on:
_cursor_off:
	    rts	

.section data

_kernel_flag: byte 1
