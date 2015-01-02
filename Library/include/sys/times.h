#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

/* User's structure for times() system call */
struct tms {
	clock_t	tms_utime;
	clock_t	tms_stime;
	clock_t	tms_cutime;
	clock_t	tms_cstime;
	clock_t	tms_etime;	/* Elapsed real time */
};

extern int times(struct tms *tms);

#endif