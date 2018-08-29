#define ide_select(x)
#define ide_deselect()

/*16bit, no altstatus/control */
#define IDE_REG_DATA		0xC0
#define IDE_REG_ERROR		0xC2
#define IDE_REG_FEATURES	0xC2
#define IDE_REG_SEC_COUNT	0xC5
#define IDE_REG_LBA_0		0xC7
#define IDE_REG_LBA_1		0xC9
#define IDE_REG_LBA_2		0xCB
#define IDE_REG_LBA_3		0xCD
#define IDE_REG_DEVHEAD		0xCD
#define IDE_REG_STATUS		0xCF
#define IDE_REG_COMMAND		0xCF

/* The data register should be accessed using inirw/otirw and similar */
#define IDE_NONSTANDARD_XFER
