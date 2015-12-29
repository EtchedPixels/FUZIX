/* Resource limits only exist on LEVEL_2 systems */

#define NRLIMIT		9

#define RLIMIT_AS	0
#define RLIMIT_CORE	1
#define RLIMIT_CPU	2
#define RLIMIT_DATA	3
#define RLIMIT_FSIZE	4
#define RLIMIT_NOFILE	5
#define RLIMIT_NPROC	6
#define RLIMIT_RSS	7
#define RLIMIT_STACK	8

typedef uint32_t rlimit_t

#define RLIM_INFINITY 0xFFFFFFFFUL

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};


extern int in_group(uint16_t gid);
extern void jobcontrol_in(struct tty *tty);
extern void jobcontrol_out(struct tty *tty);
extern int tcsetpgrp(struct tty *tty, char *data);

/* The first half of this always gets used with a constant so using a macro
   turns the whole thing into a constant 32bit comparison with a fixed
   or global register memory address */
#define limit_exceeded(l, v) \
	(udata.u_rlimit[(l)] < (v))

/* Job control requires SIGCONT is sendable to anyone in our process group.
   Untidy but we are stuck with it

   FIXME: check for any setuid funnies */
#define can_signal(p, sig) \
	((sig == SIGCONT && udata.u_ptab->p_session == (p)->session) \
	|| udata.u_ptab->p_uid == (p)->p_uid || super())

extern arg_t _setgroups(void);
extern arg_t _getgroups(void);
extern arg_t _getrlimit(void);
extern arg_t _setrlimit(void);
extern arg_t _setpgid(void);
extern arg_t _setsid(void);
extern arg_t _getsid(void);
