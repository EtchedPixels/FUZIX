#ifndef __MSX2_DOT_H__
#define __MSX2_DOT_H__

__sfr __at (0xfc) RAM_PAGE0;
__sfr __at (0xfd) RAM_PAGE1;
__sfr __at (0xfe) RAM_PAGE2;
__sfr __at (0xff) RAM_PAGE3;

#define PAGE0_BASE  (uint8_t *)0x0000
#define PAGE1_BASE  (uint8_t *)0x4000
#define PAGE2_BASE  (uint8_t *)0x8000
#define PAGE3_BASE  (uint8_t *)0xC000

#define PAGE_SIZE   0x4000

#define U_DATA		0xF000
#define U_DATA__U_PAGE	0xF002

extern int mapslot_bank1(uint8_t slot);
extern int mapslot_bank2(uint8_t slot);

extern uint8_t slotram;
extern uint8_t slotrom;
extern uint8_t machine_type;
extern uint16_t infobits;

#define MACHINE_MSX1	0
#define MACHINE_MSX2	1
#define MACHINE_MSX2P	2
#define MACHINE_MSXTR	3

#define CHARSET_MASK	(0xF)
#define CHARSET_JPN	0
#define CHARSET_INT	1
#define CHARSET_KR	2

#define INTFREQ_MASK	(1 << 7)
#define DATEFMT_MASK	(7 << 4)
#define INTFREQ_60Hz	0
#define INTFREQ_50Hz	1

#define KBDTYPE_MASK	(0xF)
#define KBDTYPE_JPN	0
#define KBDTYPE_INT	1
#define KBDTYPE_FR	2
#define KBDTYPE_UK	3
#define KBDTYPE_DIN	4
#define KBDTYPE_ES	6

#endif
