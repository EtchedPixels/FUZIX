#define PIOIDE_CS0	0x20
#define PIOIDE_R	0x40
#define PIOIDE_W	0x80

/* Bits 0 and 1 are used internally so our wiring is a bit different. We
   lose reset and CS0 but that's not a problem */
#define data	0x00
#define error	0x04
#define count	0x08
#define sec	0x0C
#define cyll	0x10
#define cylh	0x14
#define devh	0x18
#define cmd	0x1C
#define status	0x1C

