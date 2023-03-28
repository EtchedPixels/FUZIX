#define IDE_DRIVE_COUNT	1
#define IDE_8BIT_ONLY
#define IDE_IS_8BIT(x)	(1)

#define IDE_REG_INDIRECT

#define ide_reg_data 		0x10
#define ide_reg_error		0x11
#define ide_reg_features	0x11
#define ide_reg_sec_count	0x12
#define ide_reg_lba_0		0x13
#define ide_reg_lba_1		0x14
#define ide_reg_lba_2		0x15
#define ide_reg_lba_3		0x16
#define ide_reg_devhead 	0x16
#define ide_reg_command 	0x17
#define ide_reg_status		0x17

#define ide_select(x)
#define ide_deselect()
