/*
 *	We are run from the boot ROM
 */
		#include "../cpu-68000/kernel-68000.def"

		.globl __end
		.globl __bss_start
		.globl start

		.globl  cpu_type
.mri 1

vectors:
		long	udata_block+UBLOCK_SIZE
		long	start
		long	e_bus_error
		long	e_addr_error
		long	rom_illegal
		long	e_divzero
		long	e_chk
		long	e_trapv
		long	e_priv
		long	e_trace
		long	e_unimpa
		long	e_unimpf
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_uninit
		; 16-23 unused
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		; 24
		long	rc2014_vector
		long	timer_tick
		long	rc2014_vector
		long	rc2014_vector
		long	rc2014_vector
		long	rc2014_vector
		long	rc2014_vector
		long	rc2014_vector
		; 32 - TRAP #n
		long	e_trap0
		long	e_trap1
		long	e_trap2
		long	e_trap3
		long	e_trap4
		long	e_trap5
		long	e_trap6
		long	e_trap7
		long	e_trap8
		long	e_trap9
		long	e_trap10
		long	e_trap11
		long	e_trap12			; System call
		long	e_trap13
		long	e_trap14
		long	e_trap15

;
;	We are mapped at 0x00000000 on entry. Out stack pointer is not valid
;
start:
		or #$0700,sr
		; Stack
		move.l $0,a7
		; TODO copy data from ROM backup
		move.l #_etext,a0
		move.l #_data,a1
		move.l #_edata,d0
		sub.l  #_data,d0
		lsr.w #2,d0
dcopy:
		; We may copy a byte or two extra doesn't matter
		; as the BSS clear happens next
		move.l (a0)+,(a1)+
		dbra.w d0,dcopy

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

rom_illegal:
		tst.b cpuprobe
		bne cpu_type_exit
		jmp e_illegal

;
;	FIXME: doesn't tell 68020 from 68030
;	One way we could do that would be to issue a CALLM with a bogus
;	module descriptor and see if we get illegal (68030) or a format
;	exception. We can fix that when we actually need to support anything
;	bigger than a 68010 !
;
;	Based on the core code one except that we have to handle ROM
;	vectors.
;
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
