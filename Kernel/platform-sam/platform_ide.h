#define ide_select(x)
#define ide_deselect(x)

/* The numbers here look a bit weird because of the way the
   device is mapped to one Z80 port with the address high bits
   controlling what is accessed */

#define IDE_8BIT_ONLY		/* Worry about the big Atom later */

#define IDE_REG_DATA_R		0x00F6
#define	IDE_REG_DATA		IDE_REG_DATA_R
#define IDE_REG_DATA_W		0x00F7
#define IDE_REG_ERROR		0x01F6
#define IDE_REG_FEATURES	0x01F7
#define IDE_REG_SEC_COUNT	0x02F7
#define	IDE_REG_LBA_0		0x03F7
#define IDE_REG_LBA_1		0x04F7
#define IDE_REG_LBA_2		0x05F7
#define IDE_REG_LBA_3		0x06F7
#define IDE_REG_DEVHEAD		0x06F7
#define IDE_REG_STATUS		0x07F6
#define IDE_REG_COMMAND		0x07F7

/* 0xF5 bit 5 is a reset pin, setting it low resets, then set back high. We
   don't have a way to expose this to the IDE layer yet - FIXME */

#define IDE_NONSTANDARD_XFER
