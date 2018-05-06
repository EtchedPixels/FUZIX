#define IDE_DRIVE_COUNT	4
#define IDE_REG_CS1_BASE	0x60
#define IDE_HAS_RESET
#define IDE_NONSTANDARD_XFER

extern void ide_select(uint8_t drive);
#define ide_deselect()
