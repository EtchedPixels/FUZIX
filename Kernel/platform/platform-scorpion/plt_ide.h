/*
 *	NemoIDE interface
 *
 *	This is a 16bit interface with a latched data port.
 *
 *	The latch means we have to work from two ports which is ugly but
 *	at least it's not using the high bits of the address.
 */

__sfr __at 0x10 data;
__sfr __at 0x30 error;
__sfr __at 0x50 count;
__sfr __at 0x70 sec;
__sfr __at 0x90 cyll;
__sfr __at 0xB0 cylh;
__sfr __at 0xD0 devh;
__sfr __at 0xF0 cmd;
__sfr __at 0xF0 status;
__sfr __at 0xC8 dctrl;

#define IDE_REG_DATA		0x10
#define IDE_REG_DATA_LATCH	0x11

#define IDE_NONSTANDARD_XFER
