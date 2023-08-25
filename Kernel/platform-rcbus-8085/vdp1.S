
#define VDP_IRQ
#define VDP_DIRECT
#define VDP_CTRL	0x99
#define VDP_DATA	0x98

/* TODO: check the maths and what is really needed */
#define VDP_DELAY	nop
#define VDP_DELAY2	nop

#include "../dev/vdp1-8080.S"
