#define ide_select(x)
#define ide_deselect()

/* GIDE at 0x30 */
/* Don't describe the CS0 range: the firmware did all the drive reset for us */
#define IDE_REG_CS1_BASE   0x38
