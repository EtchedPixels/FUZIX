__sfr __at 0x58 data;
__sfr __at 0x01 error;
__sfr __at 0x02 count;
__sfr __at 0x03 sec;
__sfr __at 0x04 cyll;
__sfr __at 0x05 cylh;
__sfr __at 0x06 devh;
__sfr __at 0x07 cmd;
__sfr __at 0x07 status;

#define IDE_REG_DATA	0x0058

/* Altstatus and etc are 5A/5B */

/* Special banking and has a word wide interface across two ports */
#define IDE_NONSTANDARD_XFER
