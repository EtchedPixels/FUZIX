#ifndef __SYSMOD_H
#define __SYSMOD_H

/*
 *	System Interface Module
 */

struct sysinfo {
        uint8_t nbanks;		/* numbered from 0, 0 is TPA/kernel */
        uint8_t features;	/* feature flags */
#define FEATURE_AUX	1	/* Has auxist/auxost for second tty */        
#define FEATURE_TICK	2	/* Timer tick */
#define FEATURE_IODI	4	/* Disable interrupts in READ/WRITE */
#define FEATURE_CONDI	8	/* Disable interrupts in CONOUT */
#define FEATURE_DMAFD	16	/* Use common bounce buffer for floppy I/O */
#define FEATURE_DMAHD	32	/* Use common bounce buffer for hard disk I/O */
        uint8_t tickdivider;	/* Ticks per 1/10th */
        uint8_t swap;		/* CP/M drive 0-15 to use for swap, 0xFF for none */
        uint16_t common;	/* Start of common space */
        uint16_t conflags;	/* con: modifyable termios cflags bits */
        uint16_t auxflags;	/* aux: modifyable termios cflags bits */
};
        
extern void sysmod_init(void);
extern struct sysinfo *sysmod_info(void);
extern uint8_t sysmod_conost(void);
extern uint8_t sysmod_auxist(void);
extern uint8_t sysmod_auxost(void);
extern void sysmod_idle(void);
extern uint8_t sysmod_rtc_secs(void);
extern uint16_t sysmod_conconf(uint16_t cflag) __z88dk_fastcall;
extern uint16_t sysmod_auxconf(uint16_t cflag) __z88dk_fastcall;
extern uint16_t sysmod_joystick(void);

extern struct sysinfo *info;

#endif
