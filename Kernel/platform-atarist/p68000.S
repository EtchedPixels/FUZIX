/*
 *	    68000 Atari ST
 *
 *	We need pretty much nothing here as we don't do page mapping.
 */

		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl plt_reboot,plt_monitor
		.globl scroll_down,scroll_up,plot_char,clear_lines
		.globl clear_across,cursor_on,cursor_off
		.globl cursor_disable, vtattr_notify
		.globl fontdata_8x8, screenbase
		.globl uarea_block

		.globl acia_debug
.mri 1
plt_reboot:
plt_monitor:
	    or #0700,sr
	    bra plt_monitor
init_early:
	    lea.l uarea_block,a5	; udata ptr
	    move.l a5,udata_shadow
            rts

init_hardware:

	;
	;	Check for high reslution mode
	;

	cmp.b #2,$FFFF8260.w
	bne.s colour_monitor
monochrome_monitor:
	move.w #0,videomode
	move.w #0,vshift
	move.w #78,vlen
	move.b #50,videorows	; 640x400 is special
	move.b #49,videobot
	move.b #2,$FF8260
	bra set_palette

colour_monitor:
	move.w #1,videomode
	move.w #1,vshift
	move.w #156,vlen
	move.b #25,videorows	; 640x200 mode
	move.b #24,videobot
	move.b #1,$FF8260

set_palette:
	; Set our palette
	lea.l palette(pc),a0
	lea.l $FF8240,a1
	move.l (a0)+,(a1)+	; set entries 0-3 only
	move.l (a0),(a1)

mode_ok:
	move.l $42E,d0		; Top of RAM
	sub.l #64000,d0		; Frame buffer space FIXME: Ought to be 32000
	move.l d0,screenbase	; Save our video base

	; $FF820D on the STe will be zero already
	lsr.l #8,d0
	move.b d0,$FF8203	; Set address mid byte
	lsr.l #8,d0
	move.b d0,$FF8201	; Set addres high byte

        ; set system RAM size
	move.l $42E,d0		; TOS phystop
	lsr.l #8,d0			; into KBytes
	lsr.l #2,d0
	move.w d0,ramsize
	sub.w  #64,d0		; Guess for kernel
	sub.w  #64,d0		; Video memory FIXME: Ought to be 32
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

	jsr vtinit
        rts

palette:
	word $0000			; black
	word $0F00			; red
	word $00F0			; green
	word $0FFF			; white


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
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	pea vblankct
	bsr wakeup
	addq #4,sp
	movem.l (sp)+,a0/a1/a5/d0/d1
int4_out:
	rte
;
;	The level 6 handlers are autovectored
;
int_mfp1:	; DCD on the serial
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	jsr ser_dcd
	movem.l (sp)+,a0/a1/a5/d0/d1
	rte
int_mfp2:	; CTS on the seriall
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	jsr ser_cts
	movem.l (sp)+,a0/a1/a5/d0/d1
	rte
int_mfp5:	; 200Hz timer
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	jsr timer_interrupt
	bclr.b #5,$fffa11	; and clear service bit
	movem.l (sp)+,a0/a1/a5/d0/d1
	rte
int_mfp6:	; ACIA interrupts
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
acia_loop:
	jsr acia_interrupt
acia_debug:
	lea.l $fffa00,a1
	btst.b #4,$1(a1)			; Still interrupting ?
	beq acia_loop
	bclr.b #6,$11(a1)			; Clear ACIA in service
	movem.l (sp)+,a0/a1/a5/d0/d1
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
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.b $FFFA2D,d0
	move.l udata_shadow,a5	; set up the register global
	move.l d0,-(sp)
	pea.l 2
	jsr tty_inproc
	addq #8,sp
	movem.l (sp)+,a0/a1/a5/d0/d1
	rte
int_mfp14:	; MFP ring indicate
	movem.l a0/a1/a5/d0/d1,-(sp)
	move.l udata_shadow,a5	; set up the register global
	jsr ser_ri
	movem.l (sp)+,a0/a1/a5/d0/d1
	rte

; outchar: Wait for UART TX idle, then print the char in d0

outchar:
	move.w d0,-(sp)
outcharw:
	move.b $FFFA2D,d0
	btst #7,d0
	beq outcharw
	move.w (sp)+,d0
	move.b d0,$FFFA2F
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

	    ext.w d0
	    ext.w d1

	    mulu.w vwidth,d0		; 80x8 per character line
					; D0 is now a byte offset
	    move.w vshift,d2		; Get the shift per word
	    move.b d1,d3		; save X
	    and.w #1,d3			; low bit of X (it's all in words)
	    and.w #$fffe,d1		; rest of X
	    lsl.w d2,d0			; adjust for words/char
	    lsl.w d2,d1			; adjust for words/char
	    add.w d1,d0			; total offset
	    add.w d3,d0			; low bit
	    move.l screenbase,a0	; where is the display
	    add.w d0,a0			; add in the display base
	    move.w videomode,d2		; pass registers to caller
	    move.w vlen,d3		; bytes per scan line minus
					; those we write
	    rts
aligned:

;
;	Everything is pushed as a dword
;
plot_char:
	    move.b 7(sp),d0
	    move.b 11(sp),d1
	    move.l d2,-(sp)
	    move.l d3,-(sp)


plot_char_1:
	    ; Save Regs

	    bsr vaddr

	    move.w 22(sp),d0		; character
	    lsl.w #3,d0			; into font position

	    lea.l fontdata_8x8,a1	; font
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
	    add.w #1,a1
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
	    move.w #7840-1,d1
	    tst.w videomode
	    beq hires
	    move.w #1280,d0
	    move.w #7680-1,d1
hires:
	    move.l a0,a1
	    add.w d0,a1
scrollu:    move.l (a1)+,(a0)+
	    dbra d1,scrollu
	    rts

scroll_down:
	    move.l screenbase+32000,a0
	    move.w #640,d0
	    move.w #7840-1,d1
	    tst.w videomode
	    beq hiresd
	    move.w #1280,d0
	    move.w #7680-1,d1
hiresd:
	    move.l a0,a1
	    sub.w d0,a1
scrolld:    move.l -(a1),-(a0)
	    dbra d1,scrolld
	    rts

clear_lines:
	    move.b 7(sp),d0
	    move.l d2,-(sp)
	    move.l d3,-(sp)
	    moveq #0,d1
	    bsr vaddr
	    ; d2 is mode count, d3 is extra words/line, a0 is address
            move.b 19(sp),d0
	    ext.w d0
	    ; d0 is the number of character lines to wipe. We have to do 8
	    ; times that for pixels so use vwidth which is 8 x char bytes
	    ; across
	    mulu.w vwidth,d0
	    lsr.w #1,d0
	    bra wipen
wipe:
	    move.w d2,d1
wipe1:	    clr.w (a0)+
	    dbra d1,wipe1
wipen:
	    dbra d0,wipe
	    move.l (sp)+,d3
	    move.l (sp)+,d2
	    rts

clear_across:
		; TODO
	    rts	    

cursor_on:
	    move.b 7(sp),d0		; check stacking
	    move.b 11(sp),d1
	    move.l d2,-(sp)
	    move.l d3,-(sp)
	    bsr vaddr
	    lea.l cursordata,a1
	    movem.l a0/d2/d3,(a1)
;
;	Really we need to expand this per mode and to do colour
;
do_xor:
	    moveq #7,d0
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
	    cmp #0,a0
	    bne do_xor
	    move.l (sp)+,d3
	    move.l (sp)+,d2
	    rts

vtattr_notify:
	    rts	

.section data

kernel_flag: byte 1

cursordata:  word 0
	     word 0
	     word 0
	     word 0
	     word 0
	     word 0
videorows:
	     byte 0
videobot:
	     byte 0
vwidth:
	     word 640
vshift:
	     word 0
vlen:
	     word 78		; bytes to add to get to the next row
videomode:
	     word 1
vblankw:
	     byte 1
