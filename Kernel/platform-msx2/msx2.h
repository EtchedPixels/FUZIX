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

#endif
