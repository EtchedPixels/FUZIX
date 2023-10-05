__sfr __at 0xA3 data;
__sfr __at 0xA7 error;
__sfr __at 0xA8 count;
__sfr __at 0xAF sec;
__sfr __at 0xB3 cyll;
__sfr __at 0xB7 cylh;
__sfr __at 0xBB devh;
__sfr __at 0xBF cmd;
__sfr __at 0xBF status;

#define IDE_REG_DATA	0x00A3

#define IDE_NONSTANDARD_XFER
