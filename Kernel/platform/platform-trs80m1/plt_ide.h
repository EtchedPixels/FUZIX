__sfr __at 0x40 data;
__sfr __at 0x41 error;
__sfr __at 0x42 count;
__sfr __at 0x43 sec;
__sfr __at 0x44 cyll;
__sfr __at 0x45 cylh;
__sfr __at 0x46 devh;
__sfr __at 0x47 cmd;
__sfr __at 0x47 status;

/* The transfer isn't non standard but the banked memory requirements are */
#define IDE_REG_DATA	0x0040

/* The transfer isn't non standard but the banked memory requirements are */
#define IDE_NONSTANDARD_XFER
