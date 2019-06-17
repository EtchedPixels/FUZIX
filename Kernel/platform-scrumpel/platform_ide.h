#define IDE_8BIT_ONLY

#define PPIDE_BASE	0x90		/* 82C55A at 0x90 */
#define IDE_REG_INDIRECT		/* IDE registers not directly accessed */

/* PPIDE is 8bit using port A for dat and port B with inverters for control
   not the usual port C control A/B data. CS1 is not wired. Reset is not used */

/* IDE control signal to 8255 port B mapping */
#define PPIDE_A0_LINE           0x01 // Direct from 8255 to IDE interface
#define PPIDE_A1_LINE           0x02 // Direct from 8255 to IDE interface
#define PPIDE_A2_LINE           0x04 // Direct from 8255 to IDE interface
#define PPIDE_RST_LINE          0x10 // Inverter between 8255 and IDE interface
#define PPIDE_RD_LINE           0x20 // Inverter between 8255 and IDE interface
#define PPIDE_RD_BIT            5    // (1 << PPIDE_RD_BIT) = PPIDE_RD_LINE
#define PPIDE_WR_LINE           0x40 // Inverter between 8255 and IDE interface
#define PPIDE_WR_BIT            6    // (1 << PPIDE_WR_BIT) = PPIDE_WR_LINE
#define PPIDE_CS0_LINE          0x80 // Inverter between 8255 and IDE interface

/* 8255 configuration */
/* FIXME: ?? */
#define PPIDE_PPI_BUS_READ      0x90
#define PPIDE_PPI_BUS_WRITE     0x80

/* IDE register addresses */
#define ide_reg_data      (PPIDE_CS0_LINE)
#define ide_reg_error     (PPIDE_CS0_LINE | PPIDE_A0_LINE)
#define ide_reg_features  (PPIDE_CS0_LINE | PPIDE_A0_LINE)
#define ide_reg_sec_count (PPIDE_CS0_LINE | PPIDE_A1_LINE)
#define ide_reg_lba_0     (PPIDE_CS0_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define ide_reg_lba_1     (PPIDE_CS0_LINE | PPIDE_A2_LINE)
#define ide_reg_lba_2     (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A0_LINE)
#define ide_reg_lba_3     (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define ide_reg_devhead   (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define ide_reg_command   (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define ide_reg_status    (PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)

#define ide_select(x)
#define ide_deselect()
