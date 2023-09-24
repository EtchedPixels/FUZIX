#ifndef __MSX_DOT_H__
#define __MSX_DOT_H__

extern uint8_t machine_type;
extern uint16_t infobits;
extern uint8_t subslots;
extern uint8_t vdptype;

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

struct msx_map {
    uint8_t private[6];
};

extern uint8_t *map_slot1_kernel(uint8_t slotinfo) __z88dk_fastcall;
extern uint8_t *map_slot1_user(uint8_t slotinfo) __z88dk_fastcall;
extern uint8_t device_find(const uint16_t *romtab);
extern void copy_vectors(void);

extern uint16_t devtab[4][4][3];
extern uint8_t *bouncebuffer;

extern uint8_t *disk_dptr;
extern uint8_t disk_rw;
extern uint32_t disk_lba;

typedef unsigned (*xferfunc_t)(uint16_t) __z88dk_fastcall;
extern unsigned blk_xfer_bounced(xferfunc_t func, uint16_t arg);

#endif
