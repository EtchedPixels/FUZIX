#define data	0x48
#define error	0x49
#define count	0x4A
#define sec	0x4B
#define cyll	0x4C
#define cylh	0x4D
#define devh	0x4E
#define cmd	0x4F
#define status	0x4F

#define IDE_REG_DATA	0x0048

#define ide_read(x)	in(x)
#define ide_write(x,y)	out(x,y)
