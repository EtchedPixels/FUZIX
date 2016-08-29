#ifndef __PROC_H
#define __PROC_H

#include <sys/types.h>

/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process (must match value in kernel.def) */
/* The sleeping range must be together see swap.c */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_STOPPED       4    /* Stopped waiting for SIGCONT */
#define P_FORKING       5    /* In process of forking; do not mess with */
#define P_ZOMBIE        6    /* Exited. */

/* Process table entry */

struct p_tab {
    /* WRS: UPDATE kernel.def IF YOU CHANGE THIS STRUCTURE */
    uint8_t     p_status;       /* Process status: MUST BE FIRST MEMBER OF STRUCT */
    uint8_t     p_tty;          /* Process' controlling tty minor # */
    uint16_t    p_pid;          /* Process ID */
    uint16_t    p_uid;
    struct p_tab *p_pptr;      /* Process parent's table entry */
    uint16_t    p_alarm;        /* Centiseconds until alarm goes off */
    uint16_t    p_exitval;      /* Exit value */
    void *      p_wait;         /* Address of thing waited for */
    uint16_t    p_page;         /* Page mapping data */
    uint16_t    p_page2;        /* It's really four bytes for the platform */
    /* Update kernel.def if you change fields above this comment */
    /* Everything below here is overlaid by time info at exit */
    uint16_t    p_priority;     /* Process priority */
    uint32_t    p_pending;      /* Bitmask of pending signals */
    uint32_t    p_ignored;      /* Bitmask of ignored signals */
    uint32_t    p_held;         /* Bitmask of held signals */
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
    uint16_t	p_top;		/* Copy of u_top : FIXME: usize_t */
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

/* The offsets of the prof and l2 are not guaranteed to be as per this
   structure. Use this only for sizing */
struct p_tab_buffer {
    struct p_tab	p_tab;
    struct p_prof	_prof;
    struct p_level_2	_l2;
};

#endif /* __PROC_H */
