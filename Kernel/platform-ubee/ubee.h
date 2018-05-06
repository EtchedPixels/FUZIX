#ifndef _UBEE_H
#define _UBEE_H

extern uint8_t ubee_model;
#define UBEE_BASIC	0
#define UBEE_PREMIUM	1
#define UBEE_256TC	2

extern void diskprobe(void);
extern uint8_t disk_type[2];
#define DISK_TYPE_FDC	1	/* Standard FDC */
#define DISK_TYPE_FDC_D	2	/* Dreamdisc */
#define DISK_TYPE_HDC	3	/* WD fd/hd controller */

#endif
