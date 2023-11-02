#define data	0x90
#define error	0x91
#define count	0x92
#define sec	0x93
#define cyll	0x94
#define cylh	0x95
#define devh	0x96
#define cmd	0x97
#define status	0x97

#define IDE_REG_DATA	0x90

/* Due to our strange banking needs */
#define IDE_NONSTANDARD_XFER

#define ide_read(x)	in(x)
#define ide_write(x,y)	out(x,y)
