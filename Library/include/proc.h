#ifndef __PROC_H
#define __PROC_H

#include <sys/types.h>

/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process (must match value in kernel.def) */
/* The sleeping range must be together see swap.c */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_IOWAIT        4    /* Sleeping; cannot be awakened by signal */
#define P_STOPPED       5    /* Stopped waiting for SIGCONT */
#define P_FORKING       6    /* In process of forking; do not mess with */
#define P_ZOMBIE        7    /* Exited. */
#define P_NOSLEEP	8    /* Internal no-sleep state, never visible to ps */

struct __sigbits {
    uint16_t	s_pending;
    uint16_t	s_ignored;
    uint16_t	s_held;
};

/* Process table entry */

struct p_tab {
    uint8_t     p_status;       /* Process status: MUST BE FIRST MEMBER OF STRUCT */
    uint8_t	p_flags;
    uint8_t     p_tty;          /* Process' controlling tty minor # */
    uint16_t    p_pid;          /* Process ID */
    uint16_t    p_uid;
    struct p_tab *p_pptr;      /* Process parent's table entry */
    /* FIXME: uarg_t strictly but now to deal with that in user */
    unsigned int p_alarm;        /* Centiseconds until alarm goes off */
    uint16_t    p_exitval;      /* Exit value */
    void *      p_wait;         /* Address of thing waited for */
    uint16_t    p_page;         /* Page mapping data */
    uint16_t    p_page2;        /* It's really four bytes for the platform */
#if defined(__m68k__) || defined(__arm__) || defined(__ns32k__)
    void *	p_udata;
#endif
    uint16_t    p_priority;     /* Process priority */
    struct __sigbits p_sig[2];
    uint16_t    p_waitno;       /* wait #; for finding longest waiting proc */
    uint16_t    p_timeout;      /* timeout in centiseconds - 1 */
                                /* 0 indicates no timeout, 1 = expired */

/**HP**/
    char    p_name[8];
    clock_t p_time, p_utime, p_stime, p_cutime, p_cstime;
/**HP**/
    uint16_t    p_pgrp;         /* Process group */
    uint8_t     p_nice;
    uint8_t	p_event;	/* Events */
    /* FIXME: usize_t strictly */
    unsigned int p_top;		/* Copy of u_top : FIXME: usize_t */
    unsigned int p_size;	/* Process size in KB */
#ifdef __XTENSA_CALL0_ABI__
    unsigned int p_texttop;	/* Copy of u_texttop */
#endif
};

/* Followed by this structure if profiling supported */
struct p_prof {
    uint8_t     p_profscale;
    void *      p_profbuf;
    uint16_t    p_profsize;
    uint16_t    p_profoff;
};

/* Then this one if level 2 */
struct p_level_2 {
    uint16_t	p_session;
};

/* The offsets of the prof structure are not guaranteed to be as per this
   structure. Use this only for sizing */
struct p_tab_buffer {
    struct p_tab	p_tab;
    struct p_level_2	_l2;
    struct p_prof	_prof;
    void *p_timerq;
};

#endif /* __PROC_H */
