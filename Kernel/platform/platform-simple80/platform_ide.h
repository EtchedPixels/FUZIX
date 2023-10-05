#define ide_select(x)
#define ide_deselect()

/* 8bit, no altstatus/control */
#define IDE_8BIT_ONLY
#define IDE_REG_CS1_BASE   0x90

/* Due to our strange banking needs */
#define IDE_NONSTANDARD_XFER
