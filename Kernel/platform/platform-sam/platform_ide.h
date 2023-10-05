#define ide_select(x)
#define ide_deselect(x)

/*
 *	ATOM attaches the IDE controller to F5 F6 F7
 *
 *	F5 when written sets the IDE port address to use and other bits
 *	7-6: unused 5: !reset 4: !cs3 3: !cs1 2-0: address
 *
 *	CS3 selects ports 3Fx as seen by the IDE chip, CS1 selects 1Fx, both
 *	are negative logic. Both or neither is undefined
 *
 *	F6 when read causes a transaction fromthe IDE, returns the high bits
 *	and latches the low.
 *
 *	F7 when read reads the latch
 *
 *	F6 when written writes the latch
 *	F7 when written causes the 16bit wide transaction to occur using
 *	the data for the low and latch for high.
 *
 *	The ATOMlite simply makes F6 and F7 the same and requires IDE 8bit mode.
 *	That needs different handling - and has bizarre consequences for
 *	byte ordering behaviour compared to Atom
 */

#define IDE_NONSTANDARD_XFER

#define IDE_REG_INDIRECT

#define IDE_CS1			0x30
#define IDE_CS3			0x28

#define ide_reg_data		(IDE_CS1 | 0)
#define ide_reg_error		(IDE_CS1 | 1)
#define ide_reg_features	(IDE_CS1 | 1)
#define ide_reg_sec_count	(IDE_CS1 | 2)
#define ide_reg_lba_0		(IDE_CS1 | 3)
#define ide_reg_lba_1		(IDE_CS1 | 4)
#define ide_reg_lba_2		(IDE_CS1 | 5)
#define ide_reg_lba_3		(IDE_CS1 | 6)
#define ide_reg_devhead		(IDE_CS1 | 6)
#define ide_reg_status		(IDE_CS1 | 7)
#define ide_reg_command		(IDE_CS1 | 7)

#define ide_reg_altstatus	(IDE_CS3 | 6)
#define ide_reg_control		(IDE_CS3 | 7)

#include <devatom.h>

#define IDE_8BIT_ONLY		/* For the Atomlite */
#define IDE_IS_8BIT(drive)	(atom_type != ATOM_16BIT)
