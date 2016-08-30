/*
 *	Lots left to fill in
 */

		.globl trap_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl outchar
		.globl trap_monitor
		.globl udata_block

.mri 1
trap_reboot:
trap_monitor:
	    or #0700,sr
	    bra trap_monitor

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

;
;	FIXME: could be in discard if we wanted
;
init_hardware:
            ; set system RAM size(hadcode hacks for now) 
	    move.w #512,d0
	    move.w d0,ramsize
	    sub.w  #64,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

	    move.l #0,a0
	    move.w #256,d0
	    move.l #unexpected,d1
init_trap_loop:
	    move.l d1,(a0)+
	    dbra d0,init_trap_loop
	    ;
	    ; Now set the vectors we care about
	    ;
	    move.w #8,a0
	    move.l #bus_error,(a0)+
	    move.l #addr_error,(a0)+
	    move.l #illegal,(a0)+
	    move.l #divzero,(a0)+
	    move.l #chk,(a0)+
	    move.l #trapv,(a0)+
	    move.l #priv,(a0)+
	    move.l #trace,(a0)+
	    move.l #unimpa,(a0)+	; A and F line traps
	    move.l #unimpf,(a0)+
	    move.w #$80,a0
	    move.w #14,d0
trapset:
	    move.l #misctrap,(a0)+
	    dbra d0,trapset
	    move.l #trap14,(a0)+
	    move.l #trap15,(a0)+
	    move.w #$0,a0
	    move.l #uninit,$3c(a0)
	    move.l #spurious,$60(a0)
	    move.l #timer_irq,$78(a0)
	    move.l #mmu_fault,$7C(a0)

	    moveq #100,d0			; Timer on (but slow)
	    move.b d0,$00F04000
            rts

timer_irq:
	    movem.l a0-a6/d0-d7,-(a7)
	    move.l #intmsg,a0
	    bsr outstring
	    move.l udata_shadow,a5	; set up the register global
	    jsr platform_interrupt
	    move.b $00F04000,d0		; Re-enable
	    movem.l (a7)+,a0-a6/d0-d7
	    rte

intmsg:
	    asciz "IRQ"

mmu_fault:
	    movem.l a0-a6/d0-d7,-(a7)
	    move.l #mmutrap,a0
	    bsr outstring
	    movem.l (a7)+,a0-a6/d0-d7
	    rte
mmutrap:
	    asciz "MMU TRAP"
	    byte 0

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
