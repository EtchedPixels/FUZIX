#ifndef __TAPE_DOT_H
#define __TAPE_DOT_H

struct mtstatus {
    uint16_t mt_type;
    uint16_t mt_file;
    uint32_t mt_blkno;
};

#define MT_TYPE_UNKNOWN		0
#define MT_TYPE_CASSETTE	1
#define MT_TYPE_EXATRON		2

/* Add to this subset as needed for tape drive capabilities */
#define MTSTATUS	0x0600
#define MTREWIND	0x0601
#define MTSEEKF		0x0602
#define MTSEEKB		0x0603
#define MTERASE		0x0604

#endif