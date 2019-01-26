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
	;
	;	We use the ROM to do the initial set up. We must do this
	;   before we steal all the vectors
	;
	move.w #4,-(sp)
	trap #14
	and #3,d0
	move.b d0,videomode
	move.b #50,videorows	; 640x400 is special
	move.b #49,videobot
	cmpb #2,d0
	beq mode_ok			; different monitor so leave it alone
	move.b #25,videorows	; 640x200 mode
	move.b #24,videobot

	; Set our palette
	pea palette
	move.w #6,-(sp)
	trap #14
	addq #6,sp

	move.b #1,d0
mode_ok:
	move.w d0,-(sp)		; Stack resolution wanted

	move.w $42E,d0		; Top of RAM
	sub.l #32000,d0		; Frame buffer space
	move.l d0,screenbase	; Save our video base

	move.l d0,-(sp)		; physical base
	move.l d0,-(sp)		; logicalbase
	move.w #5,-(sp)		; setScreen
	trap #14
	lea 12(sp),sp

        ; set system RAM size
	move.l $42E,d0		; TOS phystop
	lsr.l #8,d0			; into KBytes
	lsr.l #2,d0
	move.w d0,ramsize
	sub.w  #64,d0		; Guess for kernel
	sub.w  #32,d0		; Video memory
	move.w d0,procmem		; guesses for now
	bsr install_vectors
	move.w #0,a0
	move.l #int2,$68(a0)		; hblank autvector
	move.l #int4,$70(a0)		; vblank autovector
	move.l #int_mfp1,$104(a0)	; DCD
	move.l #int_mfp2,$108(a0)	; CTS
	move.l #int_mfp5,$114(a0)	; 200Hz
	move.l #int_mfp6,$118(a0)	; IKBD/Midi
	move.l #int_mfp9,$124(a0)	; Tx Error
	move.l #int_mfp10,$128(a0)	; Tx Done
	move.l #int_mfp11,$12c(a0)  	; RX Error
	move.l #int_mfp12,$130(a0)	; RX Data
	move.l #int_mfp14,$138(a0)	; RI
        rts
palette:
	word $0000			; black
	word $0F00			; red
	word $00F0			; green
	word $0FFF			; white
	; We let the others go to trash


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

;
;	Interrupt vectors
;

; Hblank
int2:
	ori.w #0300,sr		; Fix up if something messes with sr
	rte
; Vblank
;	Trigger a wakeup if anyone is waiting on blanking events
;
int4:
	add.w #1,vblankct
	tst.b vblankw
	beq int4_out
	movem.l a0/a1/d0/d1,-(sp)
	pea vblankct
	bsr wakeup
	addq #4,sp
	movem.l (sp)+,a0/a1/d0/d1
int4_out:
	rte
;
;	The level 6 handlers are autovectored
;
int_mfp1:	; DCD on the serial
	movem.l a0/a1/d0/d1,-(sp)
	bsr ser_dcd
	movem.l (sp)+,a0/a1/d0/d1
	rte
int_mfp2:	; CTS on the seriall
	movem.l a0/a1/d0/d1,-(sp)
	bsr ser_cts
	movem.l (sp)+,a0/a1/d0/d1
	rte
int_mfp5:	; 200Hz timer
	movem.l a0/a1/d0/d1,-(sp)
	bsr timer_interrupt
	movem.l (sp)+,a0/a1/d0/d1
	rte
int_mfp6:	; ACIA interrupts
	movem.l a0/a1/d0/d1,-(sp)
	bsr acia_interrupt
	movem.l (sp)+,a0/a1/d0/d1
	rte
int_mfp9:	; MFP serial transmit error
	; FIXME what do we need to do to clear down ?
	rte
int_mfp10:	; MFP tx complete
	; FIXME: blocked for now using polling
	rte
int_mfp11:	; MFP rx error
	; FIXME: how to clear
	rte
int_mfp12:	; MFP receive
	movem.l a0/a1/d0/d1,-(sp)
	move.b $FFFA2D,d0
	move.l d0,-(sp)
	pea.l 2
	bsr tty_inproc
	addq #8,sp
	movem.l (sp)+,a0/a1/d0/d1
	rte
int_mfp14:	; MFP ring indicate
	movem.l a0/a1/d0/d1,-(sp)
	bsr ser_ri
	movem.l (sp)+,a0/a1/d0/d1
	rte

; outchar: Wait for UART TX idle, then print the char in d0

outchar:
	move.w d0,-(sp)
outcharw:
	move.b $FFFA13,d0
	btst #2,d0
	beq outcharw
	move.w (sp)+,d0
	move.b d0,$FFFA2D
	rts

;
;	There are 3 possible modes that we care about
;	We number then by planes-1 so the mode is our counter for dbra
;	0	640x400x1 planar
;	1	320x200x4 planar
;	3	640x200x2 planar
;
vaddr:
	    ; X in D1 Y in D0
	    ; returns address in A1, mode in D1

	    mulu.w vwidth,d0
	    mulu.w vshift,d2
	    move.b d1,d3
	    and.w #1,d3
	    and.w #$fffe,d1
	    lsl.w d2,d1
	    add.w d1,d0
	    add.w d3,d0
	    move.l screenbase,a0
	    add.w d0,a0
	    move.w videomode,d2
	    move.w vlen,d3		; bytes per scan line minus
					; those we write
	    rts
aligned:

;
;	FIXME: check how gcc pushes uint8_t's in this case and set offsets
;	properly
;
plot_char:
	    move.w 4(sp),d0
	    move.w 6(sp),d1
	    move.l d2,-(sp)
	    move.l d3,-(sp)

plot_char_1:
	    ; Save Regs

	    bsr vaddr

	    move.w 16(sp),d0		; character
	    lsl.w #3,d0			; into font position
	    lea fontdata_8x8,a1	; font
	    adda.w d0,a1
	    moveq #7,d0
;
;	Really we need to expand this per mode and to do colour
;
plotit:
	    move.w d2,d1		; count by mode
plotrow:
	    move.b (a1),(a0)
	    add.w #2,a0
	    dbra d1,plotrow
	    add.w d3,a0			; line length for mode
	    dbra d0,plotit
	    ; Restore regs
	    move.l (sp)+,d3
	    move.l (sp)+,d2
	    rts

;
;	640x400 is our special case - the bytes per line is half the other
;	modes
;
scroll_up:
	    move.l screenbase,a0
	    move.w #640,d0
	    move.w #32000-640-1,d1
	    tst.w videomode
	    beq hires
	    move.w #1280,d0
	    move.w #32000-1280-1,d1
hires:
	    move.l a0,a1
	    add.w d0,a1
scrollu:    move.l (a1)+,(a0)+
	    dbra d1,scrollu
	    rts

scroll_down:
	    move.l screenbase+32000,a0
	    move.w #640,d0
	    move.w #32000-640-1,d1
	    tst.w videomode
	    beq hiresd
	    move.w #1280,d0
	    move.w #32000-1280-1,d1
hiresd:
	    move.l a0,a1
	    sub.w d0,a1
scrolld:    move.l -(a1),-(a0)
	    dbra d1,scrolld
	    rts

clear_lines:
	    move.l 4(sp),d0
	    move.l d2,-(sp)
	    move.l d3,-(sp)
	    move.w #0,a1
	    bsr vaddr
	    ; d2 is mode count, d3 is extra words/line, a0 is address
            move.l 6(sp),d0
	    bra wipen
wipe:
	    move.w d2,d1
wipe1:	    clr.w (a0)+
	    dbra d2,wipe1
	    add.w d3,a0
wipen:
	    dbra d0,wipe
	    move.l (sp)+,d3
	    move.l (sp)+,d2
	    rts

clear_across:
	    rts	    

cursor_on:
	    move.w 4(sp),d0		; check stacking
	    move.w 6(sp),d1
	    move.l d2,-(sp)
	    move.l d3,-(sp)
	    bsr vaddr
	    moveq #7,d0
	    lea.l cursordata,a1
	    movem.l a0/d2/d3,(a1)
;
;	Really we need to expand this per mode and to do colour
;
xorit:
	    move.w d2,d1		; count by mode
xorrow:
	    eor.b #255,(a0)
	    add.w #2,a0
	    dbra d1,xorrow
	    add.w d3,a0			; line length for mode
	    dbra d0,xorit
	    move.l (sp)+,d3
	    move.l (sp)+,d2
	    rts
cursor_disable:
cursor_off:
	    move.l d2,-(sp)
	    move.l d3,-(sp)
	    lea.l cursordata,a1
	    movem.l (a1),a0/d2/d3
	    bra xorit

vtattr_notify:
	    rts	

.section data

kernel_flag: byte 1

cursordata:  word 0
	     word 0
	     word 0
	     word 0
videorows:
	     byte 0
videobot:
	     byte 0
vwidth:
	     byte 640
vshift:
	     byte 0
vlen:
	     byte 638
videomode:
	     byte 1
vblankw:
	     byte 1
