#include "../cpu-ns32k/kernel-ns32k.def"
#
#	Called with interrupts off and in supervisor mode
#

		.globl __end
		.globl __bss_start

		.text

head:
		.ascii "BOOTNS32"

		addr	head(pc),r3

		# Relocate ourselves down correctly
		movd	0x10,r0
		addr	relocate(pc),r1
		movd	0x1f000,r2
		movsd
		jump	0x1f000
#
#	This little block is run high up
#
relocate:
		movd	0x4000,r0
		movd	r3,r1
		movd	0x8000,r2
		movsd
		jump	start

#
#	And this is where we enter the code in proper location
#
start:
		# Stick our exception handler pointer in 0x80000
		# FIXME
		# Set up the ROM int and map tables.
		# addr inttab,r0
		# lprd intbase,r0
		# And modules - not that we use them
		# Second module entry uses start of RAM
		movd 0x08,r0
		lprw mod,r0
		#movd sbase,r0
		#lprd sb,r0
		# Save the CPU configuration
		sprd cfg,r0
		movd r0,_sysconfig(pc)
		addr __bss_start,r0
		addr __end,r1
wipebss:
		movqd 0,0(r0)
		addqd 4,r0
		cmpd r1,r0
		blt wipebss		# Check compare direction is right

		/* FIXME: hard coded ugly */
		addr _udata_block+1016,r0
		lprd sp,r0
		/* udata global */
		addr _udata_block,r6
		bsr init_early
		bsr init_hardware
		bsr _fuzix_main
		sprw psr,r0		# Interrupts back off
		andw 0xF7FF,r0
		lprw psr,r0
stop:		br stop

		.data

		.long 0xAABBCCDD
