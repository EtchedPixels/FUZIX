#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

/* User's structure for times() system call */
struct tms {
    time_t	tms_utime;
    time_t	tms_stime;
    time_t	tms_cutime;
    time_t	tms_cstime;
    time_t	tms_etime;	/* Elapsed real time */
};

extern int times(struct tms *tms);

#endif