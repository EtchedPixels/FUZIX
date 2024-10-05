#define PI_A0_LINE           0x01
#define PI_A1_LINE           0x02
#define PI_A2_LINE           0x04
#define PI_CS0_LINE          0x08
#define PI_CS1_LINE          0x10
#define PI_WR_LINE           0x20
#define PI_RD_LINE           0x40
#define PI_RST_LINE          0x80

/* IDE register addresses */
#define data	(PI_CS0_LINE)
#define error	(PI_CS0_LINE | PI_A0_LINE)
#define count	(PI_CS0_LINE | PI_A1_LINE)
#define sec	(PI_CS0_LINE | PI_A1_LINE | PI_A0_LINE)
#define cyll	(PI_CS0_LINE | PI_A2_LINE)
#define cylh	(PI_CS0_LINE | PI_A2_LINE | PI_A0_LINE)
#define devh	(PI_CS0_LINE | PI_A2_LINE | PI_A1_LINE)
#define cmd	(PI_CS0_LINE | PI_A2_LINE | PI_A1_LINE | PI_A0_LINE)
#define status	(PI_CS0_LINE | PI_A2_LINE | PI_A1_LINE | PI_A0_LINE)
#define altstatus (PI_CS1_LINE | PI_A2_LINE | PI_A1_LINE)
#define control   (PI_CS1_LINE | PI_A2_LINE | PI_A1_LINE | PI_A0_LINE)

#define TD_HAS_RESET	/* TODO */
