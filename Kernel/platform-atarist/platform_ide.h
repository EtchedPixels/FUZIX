/*
 *	Normal Atari Falcon, STBook and add in card IDE
 *
 *	Beware - if there is no IDE present it will bus error
 *	Beware #2 - don't probe it using F00000 or you might be fooled by
 *	an Adspeed interface and put the machine into 16Mhz mode instead. Probe
 *	one of the others.
 */

#define IDE_IS_MMIO

#define IDE_REG_DATA		0xF00001	/* 16bit at 0000/0001 */
#define IDE_REG_ERROR		0xF00005
#define IDE_REG_FEATURES	0xF00009
#define IDE_REG_SEC_COUNT	0xF00009
#define IDE_REG_LBA_0		0xF0000D
#define IDE_REG_LBA_1		0xF00011
#define IDE_REG_LBA_2		0xF00015
#define IDE_REG_LBA_3		0xF00019
#define IDE_REG_DEVHEAD		0xF00019
#define IDE_REG_COMMAND		0xF0001D
#define IDE_REG_STATUS		0xF0001D

#define IDE_REG_CONTROL		0xF00039
#define IDE_REG_ALTSTATUS	0xF00039

/* Only one controller */
#define ide_select(dev)
#define ide_deselect()

#define ide_data16		((volatile uint16_t *)0xF00000)
