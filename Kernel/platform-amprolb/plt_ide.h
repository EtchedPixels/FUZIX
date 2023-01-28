#define ide_select(x)
#define ide_deselect()

__sfr __at 0x58	data;
__sfr __at 0x59 error;
__sfr __at 0x5A count;
__sfr __at 0x5B sec;
__sfr __at 0x5C cyll;
__sfr __at 0x5D cylh;
__sfr __at 0x5E devh;
__sfr __at 0x5F status;
__sfr __at 0x5F cmd;
