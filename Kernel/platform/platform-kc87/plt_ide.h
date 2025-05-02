__sfr __at 0x58 data;
__sfr __at 0x59 error;
__sfr __at 0x5A count;
__sfr __at 0x5B sec;
__sfr __at 0x5C cyll;
__sfr __at 0x5D cylh;
__sfr __at 0x5E devh;
__sfr __at 0x5F cmd;
__sfr __at 0x5F status;

#define IDE_REG_DATA	0x0058

/* Banked xfer routines needed */
#define IDE_NONSTANDARD_XFER
