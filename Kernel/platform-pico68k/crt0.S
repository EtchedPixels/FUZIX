/*
 *	We provide a vector table for the ROM forwading. It's not in the
 *	68000 format/order though.
 */
		#include "../kernel-68000.def"

		.globl __end
		.globl __bss_start
		.globl start

		.globl  cpu_type
.mri 1

vectors:
		long	$AA556800		; boot signature
		long	start			; Start address
		jmp	e_spurious
		jmp	e_unexpected
		jmp	e_bus_error
		jmp	e_addr_error
		jmp	do_illegal
		nop				; The jump above ends up word
		jmp	e_divzero
		jmp	e_chk
		jmp	e_trapv
		jmp	e_priv
		jmp	e_trace
		jmp	e_unimpa
		jmp	e_unimpf
		jmp	e_uninit
		jmp	interrupt		; VIA and serial
		jmp	e_trap0
		jmp	e_trap1
		jmp	e_trap2
		jmp	e_trap3
		jmp	e_trap4
		jmp	e_trap5
		jmp	e_trap6
		jmp	e_trap7
		jmp	e_trap8
		jmp	e_trap9
		jmp	e_trap10
		jmp	e_trap11
		jmp	e_trap12		; System call
		jmp	e_trap13
		jmp	e_trap14
		jmp	e_trap15

start:
		lea.l udata_block+UBLOCK_SIZE,a7
		or #$0700,sr

		move.l #__bss_start,a0
		move.l #__end,d0
		sub.l a0,d0
		lsr.l #2,d0
wipebss:
		clr.l (a0)+
		dbra d0,wipebss

		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop

;
;	Based on the core code one except that we have to handle the
;	indirect vectors.
;
do_illegal:
		tst.b cpuprobe
		bne cpu_type_exit
		jmp e_illegal

cpu_type:
		move.w sr,-(sp)
		move #$2700,sr		; disable interrupts
		move.b #1,cpuprobe
		move.l a7,a1
		moveq #0,d0
		save
		chip 68010
		movec vbr,d1		; faults on a 68000
		moveq #10,d0
		chip 68020
		movec cacr,d1		; faults on a 68000 and 010
		move.b #1,cpu_has_icache
		moveq #20,d0
		chip 68040
		movec itt0,d1		; faults on 68020/30
		moveq #40,d0
		chip 68060
		movec pcr,d1		; faults on all but 68060
		moveq #60,d0
		clr.b cpuprobe
		restore
cpu_type_exit:
		move.l a1,a7
		clr.b cpuprobe
		move.b d0,cpu_has_trapvec
		move (sp)+,sr
		rts

.area data

cpuprobe:	byte 0
