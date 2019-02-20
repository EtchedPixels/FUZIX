/*
 *	DivIDE interface
 *
 *	This is a 16bit interface with a latched data port. Each read
 *	from A3 fetches a word then returns low then high etc. In the other
 *	direction it latches then writes.
 *
 *	The latch is reset to the first state by any other port access in the
 *	IDE space (so the command write sets it up nicely for us)
 */

#define ide_select(x)
#define ide_deselect()

#define IDE_DRIVE_COUNT	2

#define IDE_NONSTANDARD_XFER
#define IDE_REG_INDIRECT

#define ide_reg_data		0
#define ide_reg_error		1
#define ide_reg_features	1
#define ide_reg_sec_count	2
#define ide_reg_lba_0		3
#define ide_reg_lba_1		4
#define ide_reg_lba_2		5
#define ide_reg_lba_3		6
#define ide_reg_devhead		6
#define ide_reg_command		7
#define ide_reg_status		7

extern void divide_read_data(void);
extern void zxcf_read_data(void);
