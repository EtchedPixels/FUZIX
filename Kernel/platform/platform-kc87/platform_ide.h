#define ide_select(x)
#define ide_deselect()

#if 0 
#define IDE_REG_BASE	0x50
#define IDE_REG_CS0_FIRST
#else
#define IDE_REG_CS1_BASE	0x58
#endif

/* Banked xfer routines needed */
#define IDE_NONSTANDARD_XFER
