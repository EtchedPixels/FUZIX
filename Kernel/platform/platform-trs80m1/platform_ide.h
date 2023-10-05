#define IDE_DRIVE_COUNT		2
#define IDE_REG_CS1_BASE	0x40
#define IDE_8BIT_ONLY

#define ide_select(x)
#define ide_deselect()

/* The transfer isn't non standard but the banked memory requirements are */

#define IDE_NONSTANDARD_XFER
