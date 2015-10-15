extern uint16_t ide_base;

/* Platform provided */
extern void ide_select(uint8_t drive);
extern void ide_deselect(void);

#define IDE_REG_CS1_BASE (ide_base)
#define IDE_IS_MMIO  1		/* MMIO IDE */

