#define	data	0x10
#define error	0x11
#define count	0x12
#define sec	0x13
#define cyll	0x14
#define cylh	0x15
#define devh	0x16
#define cmd	0x17
#define status	0x17

#define IDE_REG_DATA	0x0010

#define ide_read(x)	in(x)
#define ide_write(x,y)	out(x,y)
