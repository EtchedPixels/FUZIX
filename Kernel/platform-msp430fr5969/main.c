#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include "externs.h"

uaddr_t ramtop;
uint8_t need_resched;
uint8_t last_interrupt;

struct overlay
{
	uint16_t start;
	uint16_t stop;
};

#define DECLARE_OVERLAY(n) \
	extern char __load_start_##n##_overlay; \
	extern char __load_stop_##n##_overlay

#define DEFINE_OVERLAY(n) \
	{ \
		(uint16_t) &__load_start_##n##_overlay, \
		(uint16_t) &__load_stop_##n##_overlay, \
	}

DECLARE_OVERLAY(syscall_exec16);
DECLARE_OVERLAY(syscall_fs2);
DECLARE_OVERLAY(syscall_other);

const struct overlay overlays[] = {
	DEFINE_OVERLAY(syscall_exec16),
	DEFINE_OVERLAY(syscall_fs2),
	DEFINE_OVERLAY(syscall_other)
};

const static char overlay_tab[FUZIX_SYSCALL_COUNT] = {
	0, /* __exit */ 	/* FUZIX system call 0 */
	2, /* _open */ 		/* FUZIX system call 1 */
	0, /* _close */ 	/* FUZIX system call 2 */
	3, /* _rename */ 	/* FUZIX system call 3 */
	2, /* _mknod */ 	/* FUZIX system call 4 */
	2, /* _link */ 		/* FUZIX system call 5 */
	0, /* _unlink */ 	/* FUZIX system call 6 */
	0, /* _read */ 		/* FUZIX system call 7 */
	0, /* _write */ 	/* FUZIX system call 8 */
	0, /* _lseek */ 	/* FUZIX system call 9 */
	2, /* _chdir */ 	/* FUZIX system call 10 */
	0, /* _sync */ 		/* FUZIX system call 11 */
	2, /* _access */ 	/* FUZIX system call 12 */
	2, /* _chmod */ 	/* FUZIX system call 13 */
	2, /* _chown */ 	/* FUZIX system call 14 */
	0, /* _stat */ 		/* FUZIX system call 15 */
	0, /* _fstat */ 	/* FUZIX system call 16 */
	0, /* _dup */ 		/* FUZIX system call 17 */
	0, /* _getpid */ 	/* FUZIX system call 18 */
	0, /* _getppid */ 	/* FUZIX system call 19 */
	0, /* _getuid */ 	/* FUZIX system call 20 */
	0, /* _umask */ 	/* FUZIX system call 21 */
	2, /* _getfsys */ 	/* FUZIX system call 22 */
	1, /* _execve */ 	/* FUZIX system call 23 */
	0, /* _getdirent */ /* FUZIX system call 24 */
	0, /* _setuid */ 	/* FUZIX system call 25 */
	0, /* _setgid */ 	/* FUZIX system call 26 */
	0, /* _time */ 		/* FUZIX system call 27 */
	0, /* _stime */ 	/* FUZIX system call 28 */
	0, /* _ioctl */ 	/* FUZIX system call 29 */
	0, /* _brk */ 		/* FUZIX system call 30 */
	0, /* _sbrk */ 		/* FUZIX system call 31 */
	0, /* _fork */ 		/* FUZIX system call 32 */
	3, /* _mount */ 	/* FUZIX system call 33 */
	3, /* _umount */ 	/* FUZIX system call 34 */
	0, /* _signal */ 	/* FUZIX system call 35 */
	0, /* _dup2 */ 		/* FUZIX system call 36 */
	0, /* _pause */ 	/* FUZIX system call 37 */
	0, /* _alarm */ 	/* FUZIX system call 38 */
	0, /* _kill */ 		/* FUZIX system call 39 */
	0, /* _pipe */ 		/* FUZIX system call 40 */
	0, /* _getgid */ 	/* FUZIX system call 41 */
	0, /* _times */ 	/* FUZIX system call 42 */
	2, /* _utime */ 	/* FUZIX system call 43 */
	0, /* _geteuid */ 	/* FUZIX system call 44 */
	0, /* _getegid */ 	/* FUZIX system call 45 */
	2, /* _chroot */ 	/* FUZIX system call 46 */
	2, /* _fcntl */ 	/* FUZIX system call 47 */
	2, /* _fchdir */ 	/* FUZIX system call 48 */
	2, /* _fchmod */ 	/* FUZIX system call 49 */
	2, /* _fchown */ 	/* FUZIX system call 50 */
	3, /* _mkdir */ 	/* FUZIX system call 51 */
	3, /* _rmdir */ 	/* FUZIX system call 52 */
	0, /* _setpgrp */ 	/* FUZIX system call 53 */
	2, /* _uname */ 	/* FUZIX system call 54 */
	0, /* _waitpid */ 	/* FUZIX system call 55 */
	3, /* _profil */ 	/* FUZIX system call 56 */
	3, /* _uadmin */ 	/* FUZIX systen call 57 */
	3, /* _nice */ 		/* FUZIX system call 58 */
	0, /* _sigdisp */ 	/* FUZIX system call 59 */
	2, /* _flock */ 	/* FUZIX system call 60 */
	0, /* _getpgrp */ 	/* FUZIX system call 61 */
	0, /* _sched_yield */ /* FUZIX system call 62 */
};

void platform_idle(void)
{
}

void do_beep(void)
{
}

void program_vectors(uint16_t* pageptr)
{
	/* On the MSP430, with no banking, changing processes doesn't
	 * touch the interrupt vectors. Therefore we don't need to
	 * reprogram them and this is a nop. Go us. */
}

static void maybe_load_overlay(char index)
{
	static char current_overlay = 0;
	if ((index != 0) && (current_overlay != index))
	{
		const struct overlay* o = &overlays[index-1];
		current_overlay = index;
		load_overlay(o->start, o->stop);
	}
}

void platform_discard(void)
{
	/* We're done with the start code, and the kernel's about to call
	 * _execve. So we need to make sure it's in memory. */
	maybe_load_overlay(1);
}

void load_overlay_for_syscall(void)
{
	if (udata.u_callno >= FUZIX_SYSCALL_COUNT)
		return;
	maybe_load_overlay(overlay_tab[udata.u_callno]);
}

