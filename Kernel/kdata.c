#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <netdev.h>

p_tab *init_process;
char *cmdline = (char *) CMDLINE;
uint16_t ramsize, procmem, maxproc, nproc;
uint8_t nready;
uint8_t inswap;
uint16_t runticks;
uint16_t root_dev;
uint8_t ticks_this_dsecond;
uint8_t ticks_per_dsecond;
inoptr root;
uint16_t waitno;
time_t tod;			/* Time of day */
ticks_t ticks;
int16_t acct_fh = -1;		/* Accounting file handle */

struct runload loadavg[3] = {
	{ 235, 0 },	/* 12 sets of 5 seconds per minute */
	{ 251, 0 },	/* 60 sets of 5 seconds per 5 minutes */
	{ 254, 0 }	/* 180 sets of 5 seconds per 15 minutes */
};

#ifndef CONFIG_DYNAMIC_BUFPOOL
struct blkbuf bufpool[NBUFS];
#endif

struct p_tab ptab[PTABSIZE];
struct p_tab *ptab_end;		/* Points to first byte off end */
struct p_tab *alarms;		/* Linked list of processes with timers */
struct oft of_tab[OFTSIZE];	/* Open File Table */
struct cinode i_tab[ITABSIZE];	/* In-core inode table */
struct mount fs_tab[NMOUNTS];	/* In-core mount table */

const syscall_t syscall_dispatch[FUZIX_SYSCALL_COUNT] = {
	__exit,			/* FUZIX system call 0 */
	_open,			/* FUZIX system call 1 */
	_close,			/* FUZIX system call 2 */
	_rename,		/* FUZIX system call 3 */
	_mknod,			/* FUZIX system call 4 */
	_link,			/* FUZIX system call 5 */
	_unlink,		/* FUZIX system call 6 */
	_read,			/* FUZIX system call 7 */
	_write,			/* FUZIX system call 8 */
	_lseek,			/* FUZIX system call 9 */
	_chdir,			/* FUZIX system call 10 */
	_sync,			/* FUZIX system call 11 */
	_access,		/* FUZIX system call 12 */
	_chmod,			/* FUZIX system call 13 */
	_chown,			/* FUZIX system call 14 */
	_stat,			/* FUZIX system call 15 */
	_fstat,			/* FUZIX system call 16 */
	_dup,			/* FUZIX system call 17 */
	_getpid,		/* FUZIX system call 18 */
	_getppid,		/* FUZIX system call 19 */
	_getuid,		/* FUZIX system call 20 */
	_umask,			/* FUZIX system call 21 */
	_statfs,		/* FUZIX system call 22 */
	_execve,		/* FUZIX system call 23 */
	_getdirent,		/* FUZIX system call 24 */
	_setuid,		/* FUZIX system call 25 */
	_setgid,		/* FUZIX system call 26 */
	_time,			/* FUZIX system call 27 */
	_stime,			/* FUZIX system call 28 */
	_ioctl,			/* FUZIX system call 29 */
	_brk,			/* FUZIX system call 30 */
	_sbrk,			/* FUZIX system call 31 */
	_fork,			/* FUZIX system call 32 */
	_mount,			/* FUZIX system call 33 */
	_umount,		/* FUZIX system call 34 */
	_signal,		/* FUZIX system call 35 */
	_dup2,			/* FUZIX system call 36 */
	_pause,			/* FUZIX system call 37 */
	_alarm,			/* FUZIX system call 38 */
	_kill,			/* FUZIX system call 39 */
	_pipe,			/* FUZIX system call 40 */
	_getgid,		/* FUZIX system call 41 */
	_times,			/* FUZIX system call 42 */
	_utime,			/* FUZIX system call 43 */
	_geteuid,		/* FUZIX system call 44 */
	_getegid,		/* FUZIX system call 45 */
	_chroot,		/* FUZIX system call 46 */
	_fcntl,			/* FUZIX system call 47 */
	_fchdir,		/* FUZIX system call 48 */
	_fchmod,		/* FUZIX system call 49 */
	_fchown,		/* FUZIX system call 50 */
	_mkdir,			/* FUZIX system call 51 */
	_rmdir,			/* FUZIX system call 52 */
	_setpgrp,		/* FUZIX system call 53 */
	_uname,			/* FUZIX system call 54 */
	_waitpid,		/* FUZIX system call 55 */
	_profil,		/* FUZIX system call 56 */
	_uadmin,		/* FUZIX systen call 57 */
	_nice,			/* FUZIX system call 58 */
	_sigdisp,		/* FUZIX system call 59 */
	_flock,			/* FUZIX system call 60 */
	_getpgrp,		/* FUZIX system call 61 */
	_sched_yield,		/* FUZIX system call 62 */
	_acct,			/* FUZIX system call 63 */
	_memalloc,		/* FUZIX system call 64 */
	_memfree,		/* FUZIX system call 65 */
	/* Level 2 calls and Networking calls */
#if defined(CONFIG_NET)
	_netcall,		/* Fuzix system call 66 */
#else
	_nosys,
#endif
	_ftruncate,		/* Fuzix system call 67 */
#if defined(CONFIG_LEVEL_2)
	_nosys,			/* 68-71 reserved */
	_nosys,
	_nosys,
	_nosys,
	_select,		/* FUZIX system call 72 */
	_setgroups,		/* FUZIX system call 73 */
	_getgroups,		/* FUZIX system call 74 */
	_getrlimit,		/* FUZIX system call 75 */
	_setrlimit,		/* FUZIX system call 76 */
	_setpgid,		/* FUZIX system call 77 */
	_setsid,		/* FUZIX system call 78 */
	_getsid,		/* FUZIX system call 79 */
#endif
};
