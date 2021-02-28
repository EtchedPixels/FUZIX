#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

/*******************************************
Getpid ()                        Function 18
============================================
Return Process ID Number (PID) to Caller.
********************************************/

arg_t _getpid(void)
{
	return (udata.u_ptab->p_pid);
}


/*******************************************
Getppid ()                       Function 19
============================================
Return Parent's Process ID Number (PPID).
********************************************/

arg_t _getppid(void)
{
	return (udata.u_ptab->p_pptr->p_pid);
}


/*******************************************
Getuid ()                        Function 20
============================================
Return User ID Number (UID) to Caller.
********************************************/

arg_t _getuid(void)
{
	return (udata.u_ptab->p_uid);
}


/*******************************************
Geteuid ()                       Function 44
============================================
Return Effective User ID Number (EUID).
********************************************/

arg_t _geteuid(void)
{
	return (udata.u_euid);
}


/*******************************************
Getgid ()                        Function 41
============================================
Return Group ID Number (GID) to Caller.
********************************************/

arg_t _getgid(void)
{
	return (udata.u_gid);
}


/*******************************************
Getegid ()                       Function 45
============================================
Return Effective Group ID Number (EGID).
********************************************/

arg_t _getegid(void)
{
	return (udata.u_egid);
}



/*******************************************
setuid (uid)                     Function 25        ?
int uid;

FIXME: Next API break switch to setreuid/setregid
============================================
*/

#define uid (uint16_t)udata.u_argn

arg_t _setuid(void)
{
	/* We must be super user or setting to either our real or effective
	   uid */
	if (super() || udata.u_ptab->p_uid == uid || udata.u_euid == uid) {
		/* If our effective id is root we set both. This is the
		  _POSIX_SAVED_IDS behaviiour */
		if (udata.u_euid == 0)
			udata.u_ptab->p_uid = uid;
		udata.u_euid = uid;
		return 0;
	}
	udata.u_error = EPERM;
	return -1;
}

#undef uid



/*******************************************
setgid (gid)                     Function 26        ?
int gid;

FIXME: Next API break switch to setreuid/setregid
============================================
*/

#define gid (int16_t)udata.u_argn

arg_t _setgid(void)
{
	/* We must be superuser, have the group in question is our effective
	   ore real group, or be a member of that group */
	if (super() || udata.u_gid == gid || udata.u_egid == gid 
		|| in_group(gid)) {
		if (udata.u_egid == 0)
			udata.u_gid = gid;
		udata.u_egid = gid;
		return 0;
	}
	udata.u_error = EPERM;
	return -1;
}

#undef gid



/*******************************************
time (int type, tvec)            Function 27
uint16_t type;
time_t *tvec;
============================================
Read Clock Time/Date to User Buffer.
********************************************/
#define tvec (time_t *)udata.u_argn
#define type (uint16_t)udata.u_argn1

arg_t _time(void)
{
	time_t t;
	switch (type) {
		case 0:
			rdtime(&t);
			break;
		case 1:
			t.low = ticks.full;
			t.high = 0;
			break;
		default:
			udata.u_error = EINVAL;
			return -1;
	}
	return uput(&t, tvec, sizeof(t));
}

#undef tvec
#undef type


/*******************************************
stime (tvec, type)               Function 28
time_t *tvec;
uint16_t type;
============================================
Set Clock Time (Currently unimplemented).
When active, must be SuperUser to Set Time.
********************************************/
#define tvec (time_t *)udata.u_argn
#define type (uint16_t)udata.u_argn1

arg_t _stime(void)
{
	time_t t;
	if (type != 0) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (uget(tvec, &t, sizeof(t)) || esuper())
		return -1;
	wrtime(&t);
	return (0);
}

#undef tvec
#undef type


/*******************************************
times (buf)                      Function 42        ?
char *buf;
********************************************/
#define buf (uint8_t *)udata.u_argn

arg_t _times(void)
{
	irqflags_t irq;

	irq = di();	

	uput(&udata.u_ptab->p_utime, buf, 4 * sizeof(clock_t));
	uput(&ticks, buf + 4 * sizeof(clock_t),
	     sizeof(clock_t));

	irqrestore(irq);
	return 0;
}

#undef buf




/*******************************************
brk (addr)                       Function 30
char *addr;
********************************************/
#define addr (uaddr_t)udata.u_argn

arg_t _brk(void)
{
	/* Don't allow break to be set outside of the range the platform
	   permits. For most platforms this is within 512 bytes of the
	   stack pointer

	   FIXME: if we get more complex mapping rule types then we may
	   need to make this something like  if (brk_valid(addr)) so we
	   can keep it portable */

	if (addr >= brk_limit()) {
		#if defined CONFIG_BRK_CALLS_REALLOC
			/* Claim more memory for this process. */
			if (pagemap_realloc(NULL, addr - PROGBASE))
		#endif
		{
			kprintf("%d: out of memory by %d\n", udata.u_ptab->p_pid,
				addr - brk_limit());
			panic("memory");
			udata.u_error = ENOMEM;
			return -1;
		}
	}
	if (addr < PROGBASE) {
		udata.u_error = EINVAL;
		return -1;
	}
	/* If we have done a break that gives us more room we must zero
	   the extra as we no longer guarantee it is clear already */
	if (addr > udata.u_break)
		uzero((void *)udata.u_break, addr - udata.u_break);
	udata.u_break = addr;
	return 0;
}

#undef addr



/*******************************************
sbrk (incr)                      Function 31
uint16_t incr;
********************************************/
#define incr (usize_t)udata.u_argn

arg_t _sbrk(void)
{
	uaddr_t oldbrk;

	udata.u_argn += (oldbrk = udata.u_break);
	if (_brk())		/* brk (udata.u_argn) */
		return (-1);

	return ((unsigned) oldbrk);
}

#undef incr

/*******************************************
waitpid(pid, statloc, options)    Function 55
int16_t pid;
int *statloc;
int options;
********************************************/
#define pid     udata.u_argn
#define statloc (int *)udata.u_argn1
#define options (int)udata.u_argn2

arg_t _waitpid(void)
{
	regptr ptptr p;
	int retval;
	uint8_t found;

	if (statloc && !valaddr((uint8_t *) statloc, sizeof(int))) {
		udata.u_error = EFAULT;
		return (-1);
	}

	if (pid == 0)
		pid = -udata.u_ptab->p_pgrp;
	/* Search for an exited child; */
	for (;;) {
		chksigs();
		if (udata.u_cursig) {
			udata.u_error = EINTR;
			return -1;
		}
		/* Each scan we check that we have a child, if we have a child
		   then all is good. If not then we ECHILD out */
		found = 0;
		for (p = ptab; p < ptab_end; ++p) {
			if (p->p_status && p->p_pptr == udata.u_ptab) {
				found = 1;
				if (pid == -1 || p->p_pid == pid ||
					p->p_pgrp == -pid) {
					if (p->p_status == P_ZOMBIE) {
						if (statloc)
							uputi(p->p_exitval, statloc);
						retval = p->p_pid;
						p->p_status = P_EMPTY;

						/* Add in child's cumulative time info */
						udata.u_ptab->p_cutime += p->p_utime + p->p_cutime;
						udata.u_ptab->p_cstime += p->p_stime + p->p_cstime;
						return retval;
					}
					if (p->p_event && (options & WUNTRACED)) {
						retval = (uint16_t)p->p_event << 8 | _WSTOPPED;
						p->p_event = 0;
						return retval;
					}
				}
			}
		}
		if (!found) {
			udata.u_error = ECHILD;
			return -1;
		}
		/* Nothing yet, so wait */
		if (options & WNOHANG)
			break;
		psleep(udata.u_ptab);
	}
	udata.u_error = EINTR;
	return -1;
}

#undef pid
#undef statloc
#undef options

/*******************************************
_exit (val)                       Function 0
int16_t val;
********************************************/
#define val (int16_t)udata.u_argn

arg_t __exit(void)
{
	/* Deliberately chop to 8bits */
	doexit(val << 8);
	return 0;		// ... yeah. that might not happen.
}

#undef val

/*******************************************
_fork (flags, addr)              Function 32
********************************************/
#define flags (int16_t)udata.u_argn
#define addr (uaddr_t)udata.u_argn1

arg_t _fork(void)
{
	// allocate new process
	struct p_tab *new_process;
	arg_t r;
	irqflags_t irq;

	if (flags) {
		udata.u_error = EINVAL;
		return -1;
	}

	new_process = ptab_alloc();
	if (!new_process)
		return -1;

	irq = di();
	/*
	 * We're going to run our child process next, so mark this process as
	 * being ready to run
	 *
	 * FIXME: push this down into dofork
	 */
#ifndef CONFIG_PARENT_FIRST
	udata.u_ptab->p_status = P_READY;
#endif

	/*
	 * Kick off the new process (the bifurcation happens inside here, we
	 * *MAY* returns in both the child and parent contexts, however in a
	 * non error case the child may also directly return to userspace
	 * with the return code of 0 and not return from here. Do not assume
	 * you can execute any child code reliably beyond this call.)
	 */
	r = dofork(new_process);

#ifdef DEBUG
	kprintf("Dofork %p (n %p)returns %d\n", udata.u_ptab,
		new_process, r);
	kprintf("udata.u_page %d p_page %d\n", udata.u_page,
		udata.u_ptab->p_page);
	kprintf("parent %p\n", udata.u_ptab->p_pptr);
#endif
	// if we fail this returns -1
	if (r == -1) {
		udata.u_ptab->p_status = P_RUNNING;
		pagemap_free(new_process);
		new_process->p_status = P_EMPTY;
		udata.u_error = ENOMEM;
		/* FIXME: we don't know for sure whether the error occurred
		   before or after makeproc: see bug 686 */
		nproc--;
		nready--;
	}
	irqrestore(irq);
	return r;
}

#undef flags


/*******************************************
pause (t)                         Function 37
uint16_t t;
********************************************/

#define t (int16_t)udata.u_argn

arg_t _pause(void)
{
	/* Make sure we don't set a timeout, have it expire then sleep */
	irqflags_t irq = di();
	/* 0 is a traditional "pause", n is a timeout for doing
	   sleep etc without ugly alarm hacks */
	if (t) {
		udata.u_ptab->p_timeout = t + 1;
		ptimer_insert();
	}
	psleep(0);
	irqrestore(irq);
	/* Were we interrupted ? */
	if (!t || udata.u_ptab->p_timeout > 1) {
		udata.u_error = EINTR;
		/* Our timeout is automatically cleared on syscall exit */
		return (-1);
	}
	return 0;
}

#undef t


/*******************************************
signal (sig, func)               Function 35        ?
int16_t sig;
int16_t (*func)();
********************************************/
#define sig (int16_t)udata.u_argn
#define func (int (*)(int))udata.u_argn1

arg_t _signal(void)
{
	int16_t retval;
	irqflags_t irq;
	struct sigbits *sb = udata.u_ptab->p_sig;

	if (sig < 1 || sig >= NSIGS) {
		udata.u_error = EINVAL;
		goto nogood;
	}
	if (sig > 15)
		sb++;

	irq = di();

	if (func == SIG_IGN) {
		if (sig != SIGKILL && sig != SIGSTOP)
			sb->s_ignored |= sigmask(sig);
	} else {
		if (func != SIG_DFL && !valaddr((uint8_t *) func, 1)) {
			udata.u_error = EFAULT;
			goto nogood;
		}
		sb->s_ignored &= ~sigmask(sig);
	}
	retval = (arg_t) udata.u_sigvec[sig];
	if (sig != SIGKILL && sig != SIGSTOP)
		udata.u_sigvec[sig] = func;
	/* Force recalculation of signal pending in the syscall return path */
	recalc_cursig();
	irqrestore(irq);
	
	return (retval);

nogood:
	return (-1);
}

#undef sig
#undef func

/*******************************************
sigdisp(sig, disp)		 Function 59
int16_t sig;
int16_t disp;
*******************************************/
#define sig (uint16_t)udata.u_argn
#define disp (uint16_t)udata.u_argn1

/* Implement sighold/sigrelse */
arg_t _sigdisp(void)
{
	struct sigbits *sb = udata.u_ptab->p_sig;
	if (sig < 1 || sig >= NSIGS || sig == SIGKILL || sig == SIGSTOP) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (sig > 15)
		sb++;
	if (disp == 1)
		sb->s_held |= sigmask(sig);
	else
		sb->s_held &= ~sigmask(sig);
	/* Force recalculation of signal pending in the syscall return path */
	recalc_cursig();
	return 0;
}

#undef sig
#undef disp

/*******************************************
kill (pid, sig)                  Function 39
int16_t pid;
int16_t sig;
********************************************/
#define pid (int16_t)udata.u_argn
#define sig (int16_t)udata.u_argn1

arg_t _kill(void)
{
	regptr ptptr p;
	int f = 0, s = 0;

	if (sig < 0 || sig >= NSIGS) {
		udata.u_error = EINVAL;
		return (-1);
	}

	if (pid == 0)
		udata.u_argn = -udata.u_ptab->p_pgrp;

	for (p = ptab; p < ptab_end; ++p) {
		/* No overlap here */
		if (-p->p_pgrp == pid || p->p_pid == pid) {
			f = 1;	/* Found */
			if (can_signal(p, sig)) {
				if (sig)
					ssig(p, sig);
				s = 1;	/* Signalled */
				/* Only one match possible for a process */
				if (pid > 0)
					return 0;
			}
		}
	}
	if (s)
		return 0;
	/* Not found */
	udata.u_error = ESRCH;
	/* Found but none signalled */
	if (f)
		udata.u_error = EPERM;
	return (-1);
}

#undef pid
#undef sig


/*******************************************
_alarm (dsecs)                    Function 38
uarg_t dsecs;
********************************************/
#define dsecs (uarg_t)udata.u_argn

arg_t _alarm(void)
{
	arg_t retval;

	retval = udata.u_ptab->p_alarm;
	udata.u_ptab->p_alarm = dsecs;
	ptimer_insert();
	return retval;
}

#undef secs

/*******************************************
setpgrp (void)                    Function 53
********************************************/

arg_t _setpgrp(void)
{
#ifdef CONFIG_LEVEL_2
	/* For full session management it's a shade
	   more complicated and we have the routine
	   to do the full job */
	return _setsid();
#else
	udata.u_ptab->p_pgrp = udata.u_ptab->p_pid;
	udata.u_ptab->p_tty = 0;
	return 0;
#endif	
}

/********************************************
getpgrp (void)                    Function 61
*********************************************/

arg_t _getpgrp(void)
{
	return udata.u_ptab->p_pgrp;
}

/*******************************************
_sched_yield (void)              Function 62
********************************************/

arg_t _sched_yield(void)
{
	switchout();
	return 0;
}
