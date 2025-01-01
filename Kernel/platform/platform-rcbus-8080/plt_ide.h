#define PPIDE_BASE 0x20         /* Base address of 8255A */

/* IDE control signal to 8255 port C mapping */
#define PPIDE_A0_LINE           0x01 /* Direct from 8255 to IDE interface */
#define PPIDE_A1_LINE           0x02 /* Direct from 8255 to IDE interface */
#define PPIDE_A2_LINE           0x04 /* Direct from 8255 to IDE interface */
#define PPIDE_CS0_LINE          0x08 /* Inverter between 8255 and IDE interface */
#define PPIDE_CS1_LINE          0x10 /* Inverter between 8255 and IDE interface */
#define PPIDE_WR_LINE           0x20 /* Inverter between 8255 and IDE interface */
#define PPIDE_WR_BIT            5    /* (1 << PPIDE_WR_BIT) = PPIDE_WR_LINE */
#define PPIDE_RD_LINE           0x40 /* Inverter between 8255 and IDE interface */
#define PPIDE_RD_BIT            6    /* (1 << PPIDE_RD_BIT) = PPIDE_RD_LINE */
#define PPIDE_RST_LINE          0x80 /* Inverter between 8255 and IDE interface */

/* 8255 configuration */
#define PPIDE_PPI_BUS_READ      0x92
#define PPIDE_PPI_BUS_WRITE     0x80

/* IDE register addresses */
#define data	(PPIDE_CS0_LINE)
#define error	(PPIDE_CS0_LINE | PPIDE_A0_LINE)
#define count	(PPIDE_CS0_LINE | PPIDE_A1_LINE)
#define sec	(PPIDE_CS0_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define cyll	(PPIDE_CS0_LINE | PPIDE_A2_LINE)
#define cylh	(PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A0_LINE)
#define devh	(PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define cmd	(PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define status	(PPIDE_CS0_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)
#define altstatus (PPIDE_CS1_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE)
#define control   (PPIDE_CS1_LINE | PPIDE_A2_LINE | PPIDE_A1_LINE | PPIDE_A0_LINE)

#define TD_HAS_RESET	/* TODO */
