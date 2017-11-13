#undef DEBUG			/* turn this on to enable syscall tracing */
#undef DEBUGHARDER		/* report calls to wakeup() that lead nowhere */
#undef DEBUGREALLYHARD		/* turn on getproc dumping */
#undef DEBUG_PREEMPT		/* debug pre-emption */

#include <kernel.h>
#include <tty.h>
#include <kdata.h>
#include <printf.h>
#include <audio.h>

/* psleep() puts a process to sleep on the given event.  If another
 * process is runnable, it switches out the current one and starts the
 * new one.  Normally when psleep is called, the interrupts have
 * already been disabled.   An event of 0 means a pause(), while an
 * event equal to the process's own ptab address is a wait().
 */


void psleep(void *event)
{
	irqflags_t irq = di();
#ifdef DEBUG
	kprintf("psleep(0x%p)", event);
#endif
	switch (udata.u_ptab->p_status) {
	case P_SLEEP:		// echo output from devtty happens while processes are still sleeping but in-context
	case P_STOPPED:		// coming to a halt
		nready++;	/* We will fix this back up below */
	case P_RUNNING:		// normal process
		break;
	default:
#ifdef DEBUG
	        kprintf("psleep(0x%p) -> %d:%d", event, udata.u_ptab->p_pid, udata.u_ptab->p_status);
#endif
		panic(PANIC_VOODOO);
	}

	udata.u_ptab->p_status = P_SLEEP;
	udata.u_ptab->p_wait = event;
	udata.u_ptab->p_waitno = ++waitno;
	nready--;

	/* It is safe to restore interrupts here. We have already updated the
	   process state. The worst case is that a wakeup as we switchout
	   leads us to switch out and back in, or that we wake and run
	   after other candidates - no different to it occuring after the
	   switch */
	irqrestore(irq);
	switchout();		/* Switch us out, and start another process */
	/* Switchout doesn't return in this context until we have been switched back in, of course. */
}



/* wakeup() looks for any process waiting on the event,
 * and makes it runnable
 */

void wakeup(void *event)
{
	ptptr p;
	irqflags_t irq;

#ifdef DEBUGHARDER
	kprintf("wakeup(0x%p)\n", event);
#endif
	irq = di();
	for (p = ptab; p < ptab_end; ++p) {
		if (p->p_status > P_RUNNING && p->p_wait == event) {
#ifdef DEBUG
			kprintf("wakeup: found proc 0x%p pid %d\n",
				p, p->p_pid);
#endif
			pwake(p);
		}
	}
	irqrestore(irq);
}


void pwake(ptptr p)
{
	if (p->p_status > P_RUNNING && p->p_status < P_STOPPED) {
		if (p->p_status != P_READY) {
			nready++;
			p->p_status = P_READY;
		}
		p->p_wait = NULL;
	}
}

/* Getproc returns the process table pointer of a runnable process.
 * It is actually the scheduler.  If there are none, it loops.
 * This is the only time-wasting loop in the system.
 */

ptptr getproc_nextp = &ptab[0];

#ifndef CONFIG_SINGLETASK

ptptr getproc(void)
{
	ptptr haltafter;
#ifdef DEBUG
#ifdef DEBUGREALLYHARD
	ptptr pp;
	kputs("getproc start ... ");
	for (pp = ptab; pp < ptab_end; pp++)
		kprintf("ptab[0x%p]: pid=%d uid=%d status=%d, page=0x%x\n",
			pp, pp->p_pid, pp->p_uid, pp->p_status,
			pp->p_page);
#endif
#endif

	haltafter = getproc_nextp;

	for (;;) {
		getproc_nextp++;
		if (getproc_nextp >= ptab_end) {
			getproc_nextp = ptab;
		}

		switch (getproc_nextp->p_status) {
		case P_RUNNING:
			panic(PANIC_GETPROC);
		case P_READY:
#ifdef DEBUG
			kprintf("[getproc returning %p pid=%d]\n",
				getproc_nextp, getproc_nextp->p_pid);
#endif
			return getproc_nextp;
		}
		/* Take a nap: not that it makes much difference to power on most
		   Z80 type devices */
		if (getproc_nextp == haltafter) {
			/* we have only one interrupt stack so we can't take interrupts */
			if(udata.u_ininterrupt)
				panic(PANIC_EI);
			/* yes please, interrupts on (WRS: they probably are already on?) */
			ei();
			platform_idle();
		}
	}
}

#else
/*
 *	Single tasking mode for small boxes. Keep re-selecting ourself or
 *	calling idle. When we turn zombie move back to the parent. The wait
 *	will then clean the slot and the parent continues. Most software will
 *	be blissfully unaware of its cut down environment.
 */

ptptr getproc(void)
{
	ptptr p = udata.u_ptab;

#ifdef DEBUGREALLYHARD
	kputs("getproc(");
	if (udata.u_ininterrupt)
		kputs("[IRQ]");
#endif
	while (1) {
		switch (p->p_status) {
		case P_ZOMBIE:
			/* If we died go to our parent */
#ifdef DEBUGREALLYHARD
			kprintf("Zombie: move from %p to %p\n", p,
				p->p_pptr);
#endif
			p = p->p_pptr;
		case P_READY:
			/* If we are ready run us */
			p->p_status = P_RUNNING;
		case P_RUNNING:
			/* If we are running keep running */
#ifdef DEBUGREALLYHARD
			kprintf("%p:%s:%d)\n", p, p->p_name, p->p_page);
#endif
			return p;
		default:
			/* Wait for an I/O operation to let us run, don't run
			   other tasks */
			ei();
			platform_idle();
		}
	}
}
#endif

/* Newproc fixes up the tables for the child of a fork but also for init
 * Call in the processes context!
 * This process MUST be run immediately (since it sets status P_RUNNING)
 */
void newproc(ptptr p)
{				/* Passed New process table entry */
	uint8_t *j;
	irqflags_t irq;

	irq = di();
	/* Note that ptab_alloc clears most of the entry */
	/* calculate base page of process based on ptab table offset */
	udata.u_page = p->p_page;
	udata.u_page2 = p->p_page2;

	program_vectors(&p->p_page);	/* set up vectors in new process and
					   if needed copy any common code */

	p->p_status = P_RUNNING;
	nready++;		/* runnable process count */

	p->p_pptr = udata.u_ptab;
	p->p_ignored = udata.u_ptab->p_ignored;
	p->p_tty = udata.u_ptab->p_tty;
	p->p_uid = udata.u_ptab->p_uid;
	/* Set default priority */
	p->p_priority = MAXTICKS;

	/* For systems where udata is actually a pointer or a register object */
#ifdef udata
	p->p_udata = &udata;
#endif

	udata.u_ptab = p;

	memset(&udata.u_utime, 0, 4 * sizeof(clock_t));	/* Clear tick counters */

	rdtime32(&udata.u_time);
	if (udata.u_cwd)
		i_ref(udata.u_cwd);
	if (udata.u_root)
		i_ref(udata.u_root);
	udata.u_cursig = 0;
	udata.u_error = 0;
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j) {
		if (*j != NO_FILE)
			++of_tab[*j].o_refs;
	}
	irqrestore(irq);
}

/* This allocates a new process table slot, and fills in its
 * p_pid field with a unique number.
 */
uint16_t nextpid = 0;
ptptr ptab_alloc(void)
{
	ptptr p, newp;
	irqflags_t irq;

	newp = NULL;
	udata.u_error = EAGAIN;

	irq = di();
	for (p = ptab; p < ptab_end; p++)
		if (p->p_status == P_EMPTY) {
			newp = p;
			break;
		}

	if (newp) {		/* found a slot? */
		/* zero process structure */
		memset(newp, 0, sizeof(struct p_tab));

		/* select a unique pid */
		while (newp->p_pid == 0) {
			if (nextpid++ > MAXPID)
				nextpid = 20;
			newp->p_pid = nextpid;

			for (p = ptab; p < ptab_end; p++)
				if (p->p_status != P_EMPTY
				    && p->p_pid == nextpid) {
					newp->p_pid = 0;	/* try again */
					break;
				}
		}
		newp->p_top = udata.u_top;
		if (pagemap_alloc(newp) == 0) {
			newp->p_status = P_FORKING;
			nproc++;
		} else {
			udata.u_error = ENOMEM;
			newp = NULL;
                }
                newp->p_pgrp = udata.u_ptab->p_pgrp;
                memcpy(newp->p_name, udata.u_ptab->p_name, sizeof(newp->p_name));
	}
	irqrestore(irq);
	if (newp)
		udata.u_error = 0;
	return newp;
}

/* Follow Unix tradition with load reporting (or more accurately it
   is pre-Unix from Tenex) */

void load_average(void)
{
	struct runload *r;
	static uint8_t utick;
	uint8_t i;
	uint16_t nr;

	utick++;
	if (utick < 50)
		return;

	utick = 0;

	/* Every 5 seconds */
	i = 0;
	r = &loadavg[0];
	nr = nready;

	while (i++ < 3) {
		r->average = ((((r->average - (nr << 8)) * r->exponent) +
				(((uint32_t)nr) << 16)) >> 8);
		r++;
	}
}

/* This is the clock interrupt routine.   Its job is to increment the clock
 * counters, increment the tick count of the running process, and either
 * switch it out if it has been in long enough and is in user space or mark
 * it to be switched out if in system space.  Also it decrements the alarm
 * clock of processes.
 *
 * We are running off the interrupt stack.
 */

void timer_interrupt(void)
{
	/* Increment processes and global tick counters. We can't do this
	   mid swap because we might have half of the bits for one process
	   and half of another.. */
	if (!inswap && udata.u_ptab->p_status == P_RUNNING) {
		if (udata.u_insys)
			udata.u_stime++;
		else
			udata.u_utime++;
	}

	/* Do once-per-decisecond things - this doesn't work out well on
	   boxes with 64 ticks/second.. need a better approach */
	if (++ticks_this_dsecond == ticks_per_dsecond) {
		static ptptr p;

		/* Update global time counters */
		ticks_this_dsecond = 0;
		ticks.full++;

		/* Update process alarm clocks and timeouts */
		for (p = ptab; p < ptab_end; ++p) {
			if (p->p_alarm) {
				p->p_alarm--;
				if (!p->p_alarm)
					ssig(p, SIGALRM);
			}
			if (p->p_timeout > 1) {
				p->p_timeout--;
				if (p->p_timeout == 1)
					pwake(p);
			}
		}
		updatetod();
                load_average();
#ifdef CONFIG_AUDIO
		audio_tick();
#endif
	}
#ifndef CONFIG_SINGLETASK
	/* Check run time of current process. We don't charge time while
	   swapping as the last thing we want to do is to swap a process in
	   and decide it took time to swap in so needs to go away again! */
	/* FIXME: can we kill off inint ? */
	if (!inswap && (++runticks >= udata.u_ptab->p_priority)
	    && !udata.u_insys && inint && nready > 1) {
                 need_resched = 1;
#ifdef DEBUG_PREEMPT
		kprintf("[preempt %p %d]", udata.u_ptab,
		        udata.u_ptab->p_priority);
#endif
        }
#endif
}

#ifdef DEBUG
#include "syscall_name.h"
#endif

// Fuzix system call handler
// we arrive here from syscall.s with the kernel paged in, using the kernel stack, interrupts enabled.
void unix_syscall(void)
{
	udata.u_error = 0;

	/* Fuzix saves the Stack Pointer and arguments in the
	 * Assembly Language Function handler in lowlevel.s
	 */
	if (udata.u_callno >= FUZIX_SYSCALL_COUNT) {
		udata.u_error = ENOSYS;
	} else {
#ifdef DEBUG
		kprintf("\t\tpid %d: syscall %d\t%s(%p, %p, %p)\n",
			udata.u_ptab->p_pid, udata.u_callno,
			syscall_name[udata.u_callno], udata.u_argn,
			udata.u_argn1, udata.u_argn2);
#endif
		// dispatch system call
		udata.u_retval = (*syscall_dispatch[udata.u_callno]) ();

#ifdef DEBUG
		kprintf("\t\t\tpid %d: ret syscall %d, ret %p err %p\n",
			udata.u_ptab->p_pid, udata.u_callno,
			udata.u_retval, udata.u_error);
#endif
	}
	udata.u_ptab->p_timeout = 0;

	di();
	if (runticks >= udata.u_ptab->p_priority && nready > 1) {
		/* Time to switch out? - we may have overstayed our welcome inside
		   a syscall so switch straight afterwards */
		udata.u_ptab->p_status = P_READY;
		switchout();
	}
	ei();
	chksigs();
}

void sgrpsig(uint16_t pgrp, uint8_t sig)
{
	ptptr p;
	if (pgrp) {
		for (p = ptab; p < ptab_end; ++p)
			if (p->p_pgrp == pgrp)
				ssig(p, sig);
	}
}

#ifdef CONFIG_LEVEL_2
static uint8_t dump_core(uint8_t sig)
{
        if (!(udata.u_flags & U_FLAG_NOCORE) && ((sig == SIGQUIT || sig == SIGILL || sig == SIGTRAP ||
            sig == SIGABRT || sig == SIGBUS || sig == SIGFPE ||
            sig == SIGSEGV || sig == SIGXCPU || sig == SIGXFSZ ||
            sig == SIGSYS))) {
		return sig | write_core_image();
	}
	return sig;
}
#endif                                    

/* This sees if the current process has any signals set, and handles them.
 */

static const uint32_t stopper = (1UL << SIGSTOP) | (1UL << SIGTTIN) | (1UL << SIGTTOU) | (1UL << SIGTSTP);
static const uint32_t clear = (1UL << SIGSTOP) | (1UL << SIGTTIN) | (1UL << SIGTTOU) | (1UL << SIGTSTP) |
			      (1UL << SIGCHLD) | (1UL << SIGURG) | (1UL << SIGWINCH) | (1UL << SIGIO) |
			      (1UL << SIGCONT);
uint8_t chksigs(void)
{
	uint8_t j;
	uint32_t pending = udata.u_ptab->p_pending & ~udata.u_ptab->p_held;
	int (**svec)(int) = &udata.u_sigvec[0];
	uint32_t m;

	/* Fast path - no signals pending means no work.
	   Cursig being set means we've already worked out what to do.
	 */

rescan:
	if (udata.u_cursig || !pending || udata.u_ptab->p_status == P_STOPPED)
		return udata.u_cursig;

	/* Dispatch the lowest numbered signal */
	for (j = 1; j < NSIGS; ++j) {
		svec++;
		m = sigmask(j);
		if (!(m & pending))
			continue;
		/* This is more complex than in V7 - we have multiple
		   behaviours plus core dump */
		if (*svec == SIG_DFL) {
		        /* SIGSTOP can't be ignored and puts the process into
		           P_STOPPED state when it is ready to handle the signal.
		           Annoyingly right now we have to context switch to the task
		           in order to stop it in the right place. That would be nice
		           to fix */
		        if (m & stopper) {
				/* Don't allow us to race SIGCONT */
				irqflags_t irq = di();
				/* FIXME: can we ever end up here not in READY/RUNNING ? */
				nready--;
				udata.u_ptab->p_status = P_STOPPED;
				udata.u_ptab->p_event = j;
				udata.u_ptab->p_pending &= ~m;	// unset the bit
				irqrestore(irq);
				switchout();
				/* Other things may have happened */
				goto rescan;
	                }

			/* The signal is being handled, so clear it even if
			   we are exiting (otherwise we'll loop in
			   chksigs) */
			udata.u_ptab->p_pending &= ~m;

	                if ((m & clear) || udata.u_ptab->p_pid == 1) {
			/* SIGCONT is subtle - we woke the process to handle
			   the signal so ignoring here works fine */
				continue;
			}
#ifdef DEBUG
			kprintf("process terminated by signal %d\n", j);
#endif
			/* We may have marked ourselves as asleep and
			   then been caught by the chksigs when we tried
			   to task switch into bed. In that case we need
			   to put the process back in running state */
			if (udata.u_ptab->p_status == P_SLEEP) {
				udata.u_ptab->p_status = P_RUNNING;
				nready++;
			}
			doexit(dump_core(j));
		} else if (*svec != SIG_IGN) {
			/* Arrange to call the user routine at return */
			udata.u_ptab->p_pending &= ~m;	// unset the bit
#ifdef DEBUG
			kprintf("about to process signal %d\n", j);
#endif
			udata.u_cursig = j;
			break;
		}
	}
	return udata.u_cursig;
}

/*
 *	Send signal, avoid touching uarea
 */
/* SDCC bug #2472: SDCC generates hideous code for this function due to bad
   code generation when masking longs. Not clear we can do much about it but
   file a bug */
void ssig(ptptr proc, uint8_t sig)
{
	uint32_t sigm;
	irqflags_t irq;

	sigm = sigmask(sig);

	irq = di();

	if (proc->p_status != P_EMPTY) {	/* Presumably was killed just now */
                /* SIGCONT has an unblockable effect */
		if (sig == SIGCONT) {
			/* You can never send yourself a SIGCONT when you are stopped */
			if (proc->p_status == P_STOPPED) {
			        proc->p_status = P_READY;
			        proc->p_event = 0;
			        nready++;
			}
			/* CONT discards pending stops */
			proc->p_pending &= ~((1L << SIGSTOP) | (1L << SIGTTIN) |
					     (1L << SIGTTOU) | (1L << SIGTSTP));
		}
		/* STOP discards pending conts */
		if (sig >= SIGSTOP && sig <= SIGTTOU)
			proc->p_pending &= ~(1L << SIGCONT);

		/* Routine signal behaviour */
		if (!(proc->p_ignored & sigm)) {
			/* Don't wake for held signals */
			if (!(proc->p_held & sigm)) {
				switch (proc->p_status) {
				case P_SLEEP:
					proc->p_status = P_READY;
					nready++;
				default:;
				}
				proc->p_wait = NULL;
			}
			proc->p_pending |= sigm;
		}
	}
	irqrestore(irq);
}

#ifdef CONFIG_ACCT
void acctexit(ptptr p)
{
	struct oft *oftp;
	inoptr ino;

	if (acct_fh == -1)
		return;

	oftp = &of_tab[acct_fh];
	ino = oftp->o_inode;

	/* Reuse the field before we write out accounting data */
	udata.u_mask = p->p_uid;
	/* More useful information is u_top */
	udata.u_break = udata.u_top;

	udata.u_sysio = true;
	udata.u_base = (char *) &udata.u_mask;
	udata.u_count = 48;	/* Includes some spare for expansion */

	/* Append to the end of the file */
	oftp->o_ptr = ino->c_node.i_size;
	writei(ino, 0);
}
#endif

/* Perform the terminal process signalling */
/* FIXME: why return a value - we don't use it */
static int signal_parent(ptptr p)
{
        if (p->p_ignored & (1UL << SIGCHLD)) {
		/* POSIX.1 says that SIG_IGN for SIGCHLD means don't go
		   zombie, just clean up as we go */
		udata.u_ptab->p_status = P_EMPTY;
                return 0;
	}
	ssig(p, SIGCHLD);
	/* Wake up a waiting parent, if any. */
	wakeup((char *)p);
	udata.u_ptab->p_status = P_ZOMBIE;
	return 1;
}

void doexit(uint16_t val)
{
	int16_t j;
	ptptr p;
	irqflags_t irq;

#ifdef DEBUG
	kprintf("process %d exiting %d\n", udata.u_ptab->p_pid, val);

	kprintf
	    ("udata.u_page %u, udata.u_ptab %p, udata.u_ptab->p_page %u\n",
	     udata.u_page, udata.u_ptab, udata.u_ptab->p_page);
#endif
	if (udata.u_ptab->p_pid == 1)
		panic(PANIC_KILLED_INIT);

	sync();		/* Not necessary, but a good idea. */

	irq = di();

	/* We are exiting, hold all signals  (they will never be
	   delivered). If we don't do this we might take a signal
	   while exiting which would be ... unfortunate */
	udata.u_ptab->p_held = 0xFFFFFFFFUL;
	udata.u_cursig = 0;

	/* Discard our memory before we blow away and reuse the memory */
	pagemap_free(udata.u_ptab);

	for (j = 0; j < UFTSIZE; ++j) {
		if (udata.u_files[j] != NO_FILE)
			doclose(j);
	}


	udata.u_ptab->p_exitval = val;

	i_deref(udata.u_cwd);
	i_deref(udata.u_root);

	/* Stash away child's execution tick counts in process table,
	 * overlaying some no longer necessary stuff.
	 *
	 * Pedantically POSIX says we should do this at the point of wait()
	 */
	udata.u_utime += udata.u_cutime;
	udata.u_stime += udata.u_cstime;
	memcpy(&(udata.u_ptab->p_priority), &udata.u_utime,
	       2 * sizeof(clock_t));

	for (p = ptab; p < ptab_end; ++p) {
		if (p->p_status == P_EMPTY || p == udata.u_ptab)
			continue;
		/* Set any child's parents to init */
		if (p->p_pptr == udata.u_ptab) {
			p->p_pptr = ptab;	/* ptab is always init */
			/* Suppose our child is a zombie and init has
			   SIGCLD blocked */
		        if (ptab[0].p_ignored & (1UL << SIGCHLD)) {
				p->p_status = P_EMPTY;
			} else {
				ssig(&ptab[0], SIGCHLD);
				wakeup(&ptab[0]);
			}
		}
		/* Send SIGHUP to any pgrp members and remove
		   them from our pgrp */
                if (p->p_pgrp == udata.u_ptab->p_pid) {
			p->p_pgrp = 0;
			ssig(p, SIGHUP);
			ssig(p, SIGCONT);
		}
	}
	tty_exit();
	irqrestore(irq);
#ifdef DEBUG
	kprintf
	    ("udata.u_page %u, udata.u_ptab %p, udata.u_ptab->p_page %u\n",
	     udata.u_page, udata.u_ptab, udata.u_ptab->p_page);
#endif
#ifdef CONFIG_ACCT
	acctexit(p);
#endif
        udata.u_page = 0xFFFFU;
        udata.u_page2 = 0xFFFFU;
        signal_parent(udata.u_ptab->p_pptr);
	nready--;
	nproc--;

	switchin(getproc());
	panic(PANIC_DOEXIT);
}

void panic(char *deathcry)
{
	kputs("\r\npanic: ");
	kputs(deathcry);
	trap_monitor();
}

/* We put this here so that we can blow the start.c code away on exec
   eventually, but still manage to get the panic() to happen if it fails */
void exec_or_die(void)
{
	kputs("Starting /init\n");
	platform_discard();
	_execve();
	panic(PANIC_NOINIT);	/* BIG Trouble if we Get Here!! */
}
