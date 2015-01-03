#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#undef DEBUG

/*******************************************
Getpid ()                        Function 18
============================================
Return Process ID Number (PID) to Caller.
********************************************/

int16_t _getpid(void)
{
	return (udata.u_ptab->p_pid);
}


/*******************************************
Getppid ()                       Function 19
============================================
Return Parent's Process ID Number (PPID).
********************************************/

int16_t _getppid(void)
{
	return (udata.u_ptab->p_pptr->p_pid);
}


/*******************************************
Getuid ()                        Function 20
============================================
Return User ID Number (UID) to Caller.
********************************************/

int16_t _getuid(void)
{
	return (udata.u_ptab->p_uid);
}


/*******************************************
Geteuid ()                       Function 44
============================================
Return Effective User ID Number (EUID).
********************************************/

int16_t _geteuid(void)
{
	return (udata.u_euid);
}


/*******************************************
Getgid ()                        Function 41
============================================
Return Group ID Number (GID) to Caller.
********************************************/

int16_t _getgid(void)
{
	return (udata.u_gid);
}


/*******************************************
Getegid ()                       Function 45
============================================
Return Effective Group ID Number (EGID).
********************************************/

int16_t _getegid(void)
{
	return (udata.u_egid);
}



/*******************************************
setuid (uid)                     Function 25        ?
int uid;
============================================
Set User ID Number (UID) of Process.  Must
be SuperUser or owner, else Error (EPERM).
********************************************/
#define uid (int)udata.u_argn

int16_t _setuid(void)
{
	if (super() || udata.u_ptab->p_uid == uid) {
		udata.u_ptab->p_uid = uid;
		udata.u_euid = uid;
		return (0);
	}
	udata.u_error = EPERM;
	return (-1);
}

#undef uid



/*******************************************
setgid (gid)                     Function 26        ?
int gid;
============================================
Set Group ID Number (GID).  Must be Super-
User or in Group to Set, else Error (EPERM).
********************************************/
#define gid (int16_t)udata.u_argn

int16_t _setgid(void)
{
	if (super() || udata.u_gid == gid) {
		udata.u_gid = gid;
		udata.u_egid = gid;
		return (0);
	}
	udata.u_error = EPERM;
	return (-1);
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

int16_t _time(void)
{
	time_t t;
	switch (type) {
		case 0:
			rdtime(&t);
			uput(&t, tvec, sizeof(t));
			return (0);
		case 1:
			uput(&t.low, &ticks.full, sizeof(ticks));
			uzero(&t.high, sizeof(t.high));
			return 0;
		default:
			udata.u_error = EINVAL;
			return -1;
	}
}

#undef tvec
#undef type


/*******************************************
stime (tvec)                     Function 28
time_t *tvec;
============================================
Set Clock Time (Currently unimplemented).
When active, must be SuperUser to Set Time.
********************************************/
#define tvec (time_t *)udata.u_argn
#define type (uint16_t)udata.u_argn1

int16_t _stime(void)
{
	time_t t;
	if (type != 0) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (uget(&t, tvec, sizeof(t)) || esuper())
		return -1;
	wrtime(&t);
	return (-1);
}

#undef tvec
#undef type


/*******************************************
times (buf)                      Function 42        ?
char *buf;
********************************************/
#define buf (char *)udata.u_argn

int16_t _times(void)
{
	irqflags_t irq;

	irq = di();	

	uput(&udata.u_utime, buf, 4 * sizeof(clock_t));
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
#define addr (char *)udata.u_argn

int16_t _brk(void)
{
	/* FIXME: when we start building binaries with the stack embedded in them
	   they will need a different test.. */
	/* Don't allow break to be set past user's stack pointer */
    /*** St. Nitschke allow min. of 512 bytes for Stack ***/
	if (addr >= (char *) (udata.u_syscall_sp) - 512) {
		kprintf("%d: out of memory\n", udata.u_ptab->p_pid);
		udata.u_error = ENOMEM;
		return (-1);
	}
	udata.u_break = (unsigned) addr;
	return (0);
}

#undef addr



/*******************************************
sbrk (incr)                      Function 31
uint16_t incr;
********************************************/
#define incr (uint16_t)udata.u_argn

int16_t _sbrk(void)
{
	unsigned oldbrk;

	udata.u_argn += (oldbrk = udata.u_break);
	if ((unsigned) udata.u_argn < oldbrk)
		return (-1);
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

int16_t _waitpid(void)
{
	ptptr p;
	int retval;

	if (statloc && !valaddr((char *) statloc, sizeof(int))) {
		udata.u_error = EFAULT;
		return (-1);
	}

	/* See if we have any children. */
	for (p = ptab; p < ptab_end; ++p) {
		if (p->p_status && p->p_pptr == udata.u_ptab
		    && p != udata.u_ptab)
			goto ok;
	}
	udata.u_error = ECHILD;
	return (-1);
      ok:
	if (pid == 0)
		pid = -udata.u_ptab->p_pgrp;
	/* Search for an exited child; */
	for (;;) {
		chksigs();
		if (udata.u_cursig) {
			udata.u_error = EINTR;
			return (-1);
		}
		for (p = ptab; p < ptab_end; ++p) {
			if (p->p_status == P_ZOMBIE
			    && p->p_pptr == udata.u_ptab) {
				if (pid == -1 || p->p_pid == pid
				    || p->p_pgrp == -pid) {
					if (statloc)
						uputw(p->p_exitval,
						      statloc);

					retval = p->p_pid;
					p->p_status = P_EMPTY;

					/* Add in child's time info.  It was stored on top */
					/* of p_priority in the childs process table entry. */
					udata.u_cutime += ((clock_t *)p->p_priority)[0];
					udata.u_cstime += ((clock_t *)p->p_priority)[1];
					return retval;
				}
			}
		}
		/* Nothing yet, so wait */
		if (options & WNOHANG)
			break;
		psleep(udata.u_ptab);
	}
	return 0;
}

#undef pid
#undef statloc
#undef options

/*******************************************
_exit (val)                       Function 0
int16_t val;
********************************************/
#define val (int16_t)udata.u_argn

int16_t __exit(void)
{
	doexit(val, 0);
	return 0;		// ... yeah. that might not happen.
}

#undef val

/*******************************************
fork ()                          Function 32
********************************************/

int16_t _fork(void)
{
	// allocate new process
	struct p_tab *new_process;
	int16_t r;
	irqflags_t irq;

	new_process = ptab_alloc();
	if (!new_process)
		return -1;

	irq = di();
	// we're going to run our child process next, so mark this process as being ready to run
	udata.u_ptab->p_status = P_READY;
	// kick off the new process (the bifurcation happens inside here, we returns in both 
	// the child and parent contexts)
	r = dofork(new_process);
#ifdef DEBUG
	kprintf("Dofork %x (n %x)returns %d\n", udata.u_ptab,
		new_process, r);
	kprintf("udata.u_page %d p_page %d\n", udata.u_page,
		udata.u_ptab->p_page);
	kprintf("parent %x\n", udata.u_ptab->p_pptr);
#endif
	// if we fail this returns -1
	if (r == -1) {
		udata.u_ptab->p_status = P_RUNNING;
		pagemap_free(new_process);
		new_process->p_status = P_EMPTY;
		udata.u_error = ENOMEM;
		nproc--;
		nready--;
	}
	irqrestore(irq);

	return r;
}



/*******************************************
pause (t)                         Function 37
uint16_t t;
********************************************/

#define t (int16_t)udata.u_argn

int16_t _pause(void)
{
	/* 0 is a traditional "pause", n is a timeout for doing
	   sleep etc without ugly alarm hacks */
	if (t)
		udata.u_ptab->p_timeout = t + 1;
	psleep(0);
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
#define func (int16_t (*)())udata.u_argn1

int16_t _signal(void)
{
	int16_t retval;
	irqflags_t irq;

	if (sig < 1 || sig >= NSIGS) {
		udata.u_error = EINVAL;
		goto nogood;
	}

	irq = di();

	if (func == SIG_IGN) {
		if (sig != SIGKILL && sig != SIGSTOP)
			udata.u_ptab->p_ignored |= sigmask(sig);
	} else {
		if (func != SIG_DFL && !valaddr((char *) func, 1)) {
			udata.u_error = EFAULT;
			goto nogood;
		}
		udata.u_ptab->p_ignored &= ~sigmask(sig);
	}
	retval = (int) udata.u_sigvec[sig];
	if (sig != SIGKILL && sig != SIGSTOP)
		udata.u_sigvec[sig] = func;
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
#define sig (int16_t)udata.u_argn
#define disp (int16_t)udata.u_argn1

/* Implement sighold/sigrelse */
int16_t _sigdisp(void)
{
	if (sig < 1 || sig >= NSIGS || sig == SIGKILL || sig == SIGSTOP) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (disp == 1)
		udata.u_ptab->p_held |= sigmask(sig);
	else
		udata.u_ptab->p_held &= ~sigmask(sig);
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

int16_t _kill(void)
{
	ptptr p;
	int f = 0, s = 0;

	if (sig < 0 || sig > 15) {
		udata.u_error = EINVAL;
		return (-1);
	}

	if (pid == 0)
		udata.u_argn = -udata.u_ptab->p_pgrp;

	for (p = ptab; p < ptab_end; ++p) {
		/* No overlap here */
		if (-p->p_pgrp == pid || p->p_pid == pid) {
			f = 1;	/* Found */
			if (udata.u_ptab->p_uid == p->p_uid || super()) {
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
_alarm (secs)                    Function 38
uint16_t secs;
********************************************/
#define secs (int16_t)udata.u_argn

int16_t _alarm(void)
{
	int16_t retval;

	retval = udata.u_ptab->p_alarm / 10;
	udata.u_ptab->p_alarm = secs * 10;
	return (retval);
}

#undef secs

/*******************************************
setpgrp (void)                    Function 53
********************************************/

int16_t _setpgrp(void)
{
	udata.u_ptab->p_pgrp = udata.u_ptab->p_pid;
	return (0);
}
