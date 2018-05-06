#ifndef _UBEE_H
#define _UBEE_H

extern uint8_t ubee_model;
#define UBEE_BASIC	0
#define UBEE_PREMIUM	1
#define UBEE_256TC	2
#define UBEE_BASIC_MONO	3

extern uint8_t ubee_parallel;
#define UBEE_PARALLEL_LPR	0
#define UBEE_PARALLEL_JOYSTICK	1
#define UBEE_PARALLEL_BEETALKER	2
#define UBEE_PARALLEL_BEETHOVEN	3
#define UBEE_PARALLEL_DAC	4
#define UBEE_PARALLEL_COMPUMUSE	5

extern void diskprobe(void);
extern uint8_t disk_type[2];
#define DISK_TYPE_FDC	1	/* Standard FDC */
#define DISK_TYPE_FDC_D	2	/* Dreamdisc */
#define DISK_TYPE_HDC	3	/* WD fd/hd controller */

/* Turn on the turbo if fitted. There is no way to check if this worked
   except clever timing tricks so we trust the user */
extern void engage_warp_drive(void);

#endif
