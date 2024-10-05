#define data	0x58
#define error	0x01
#define count	0x02
#define sec	0x03
#define cyll	0x04
#define cylh	0x05
#define devh	0x06
#define cmd	0x07
#define status	0x07

#define IDE_REG_DATA	0x0058

/* Altstatus and etc are 5A/5B */

#define ide_read(x)	in(x)
#define ide_write(x,y)	out(x,y)
