__sfr __banked __at 0xFD08 data;
__sfr __banked __at 0xFD09 error;
__sfr __banked __at 0xFD0A count;
__sfr __banked __at 0xFD0B sec;
__sfr __banked __at 0xFD0C cyll;
__sfr __banked __at 0xFD0D cylh;
__sfr __banked __at 0xFD0E devh;
__sfr __banked __at 0xFD0F cmd;
__sfr __banked __at 0xFD0F status;

#define IDE_REG_DATA	0xFD08

#define IDE_NONSTANDARD_XFER
