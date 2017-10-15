	.arch i8086,jumps
	.code16
	.att_syntax prefix
	.data

	.global udata
	.global kstack_top
	.global istack_top

udata:
	.bss 512
kstack_top:
	.bss 256
istack_top:

	.text

	.global init_early
	.global init_hardware
	.global program_vectors
	.global trap_monitor
	.global trap_reboot
	.global kernel_ds

	.global bioshd_param
	.global bioshd_read
	.global bioshd_write
	.global bioshd_reset
	.global biosdata_read
	.global equipment_word


init_early:
	/* Dig useful information out of the BIOS */
	int $11
	mov %ax,equipment_word
	int $12
	mov %ax,ramsize
	ret
init_hardware:
	ret
program_vectors:
	ret
trap_monitor:
trap_reboot:
	jmp trap_monitor

/* Returns the BIOS parameter data CX:DX as a long DX:AX to the C code or
   FFFF if we don't get the right answer

   Things to note this being PC BIOS land
	- bl return is not reliable for floppies (nor sometimes %cx)
	- es:di is not reliably set by the BIOS
	- you must do an int 13 ah=0x01 after this call on PS/2 model 30
	- some junk leaves interrupts disabled after this call
	- Leading Edge BIOSes may trash di/si/bp/ds/es !
	- Some Compaq BIOSes will make stuff up for drive numbers above
	  the last one we use

    long bioshd_param(int drive)
*/
bioshd_param:
	pushw	%bp
	movw	%sp,%bp
	movw	$0x0800,%ax		/* get drive parameters */
	movw	4(%bp),%dx
	pushf
	stc
	pushw	%di
	pushw	%si
	pushw	%bp
	pushw	%ds
	int	$13
	popw	%ds
	movw	%ds,%ax
	movw	%ax,%es
	popw	%bp
	popw	%si
	popw	%di
	jc	bioshd_fail
	popf
	pushw	%cx
	pushw	%dx
	movw	4(%bp),%dx		/* drive number */
	movb	$1,%ah
	int	$13
	popw	%dx
	popw	%ax
	popw	%bp
	ret
bioshd_fail:
	popf
	popw	%bp
	mov	$0xFF,%ax
	mov	%ax,%dx
	ret

bioshd_status:
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%dx
	movb	$1,%ah
	int	$13
	popw	%bp
	ret
	
bioshd_reset:
	xorb	%ah,%ah
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%dx
	int	$13
	popw	%bp
	ret

/*
 *	Attempt a block read
 *
 *	Guess what this is also horribly buggy on lots of systems too
 *	Buffer must be even on some AMI BIOSen (1990/1) so we need to
 *	handle direct I/O with care and also FIXME align buffer cache
 *
 *	Some bioses destroy dx, and don't always handle carry right so we
 *	must call with stc first
 *
 *	The original IBM PC-AT BIOS doesn't correctly handle interrupt
 *	disables. For now we just treat that as busted - use a newer BIOS,
 *	you've had 30 years to upgrade it!
 *
 *	bioshd_read(uint16_t cylsec, uint8_t drive, uint8_t head,
 *			uint16_t seg, uint16_t addr, uint16_t secs)
 */
bioshd_read:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	movw	4(%bp),%cx
	movw	6(%bp),%dx
	movw	8(%bp),%ax
	movw	%ax,%es
	movw	10(%bp),%bx
	movw	12(%bp),%ax
	movb	$2,%ah
	stc
	int	$13
	jc	read_fail
	movw	$0,%dx
read_end:
	popw	%es
	popw	%bp
	ret
read_fail:
	movw	$0xff,%dx
	jmp	read_end
	

bioshd_write:
	pushw	%bp
	movw	%sp,%bp
	pushw	%es
	movw	4(%bp),%cx
	movw	6(%bp),%dx
	movw	8(%bp),%ax
	movw	%ax,%es
	movw	10(%bp),%bx
	movw	12(%bp),%ax
	movb	$3,%ah
	stc
	int	$13
	jc	read_fail
	movw	$0,%dx
	popw	%es
	popw	%bp
	ret

biosdata_read:
	pushw	%bp
	movw	%sp,%bp
	pushw	%di
	movw	$0x40,%ax
	movw	%ax,%es
	movw	4(%bp),%di
	movb	(%di),%al
	cbw
	movw	%ds,%bx
	movw	%bx,%es
	popw	%di
	popw	%bp
	ret

kernel_ds:
	.word 0

	.data
equipment_word:
	.word 0
