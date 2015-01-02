#undef DEBUG			/* turn this on to enable syscall tracing */
#undef DEBUGHARDER		/* report calls to wakeup() that lead nowhere */
#undef DEBUGREALLYHARD		/* turn on getproc dumping */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

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
	kprintf("psleep(0x%x)", event);	/* WRS */
#endif

	switch (udata.u_ptab->p_status) {
	case P_SLEEP:		// echo output from devtty happens while processes are still sleeping but in-context
		nready++;	/* We will fix this back up below */
	case P_RUNNING:	// normal process
		break;
	default:
		panic("psleep: voodoo");
	}

	if (!event)
		udata.u_ptab->p_status = P_PAUSE;
	else if (event == (char *) udata.u_ptab)
		udata.u_ptab->p_status = P_WAIT;
	else
		udata.u_ptab->p_status = P_SLEEP;
	udata.u_ptab->p_wait = event;
	udata.u_ptab->p_waitno = ++waitno;
	nready--;

	/* FIXME: we don't want to restore interrupts here, but what
	   is the consequence */
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
	kprintf("wakeup(0x%x)\n", event);
#endif
	irq = di();
	for (p = ptab; p < ptab + maxproc; ++p) {
		if (p->p_status > P_RUNNING && p->p_wait == event) {
#ifdef DEBUG
			kprintf("wakeup: found proc 0x%x pid %d\n",
				p, p->p_pid);
#endif
			p->p_status = P_READY;
			p->p_wait = NULL;
			nready++;
		}
	}
	irqrestore(irq);
}


void pwake(ptptr p)
{
	if (p->p_status > P_RUNNING) {
		p->p_status = P_READY;
		p->p_wait = NULL;
		nready++;
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
	kputs("getproc start ... ");
	for (pp = ptab; pp < ptab + maxproc; pp++)
		kprintf("ptab[0x%x]: pid=%d uid=%d status=%d, page=0x%x\n",
			pp, pp->p_pid, pp->p_uid, pp->p_status,
			pp->p_page);
	*/
#endif
#endif
	    /* yes please, interrupts on. */
	    ei();

	haltafter = getproc_nextp;

	for (;;) {
		getproc_nextp++;
		if (getproc_nextp >= ptab + maxproc) {
			getproc_nextp = ptab;
		}

		switch (getproc_nextp->p_status) {
		case P_RUNNING:
			panic("getproc: extra running");
		case P_READY:
#ifdef DEBUG
			kprintf("[getproc returning %x pid=%d]\n",
				getproc_nextp, getproc_nextp->p_pid);
#endif
			return getproc_nextp;
		}
		/* Take a nap: not that it makes much difference to power on most
		   Z80 type devices */
		if (getproc_nextp == haltafter) {
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

	/* yes please, interrupts on. */
	ei();

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
			kprintf("Zombie: move from %x to %x\n", p,
				p->p_pptr);
#endif
			p = p->p_pptr;
		case P_READY:
			/* If we are ready run us */
			p->p_status = P_RUNNING;
		case P_RUNNING:
			/* If we are running keep running */
#ifdef DEBUGREALLYHARD
			kprintf("%x:%s:%d)\n", p, p->p_name, p->p_page);
#endif
			return p;
		default:
			/* Wait for an I/O operation to let us run, don't run
			   other tasks */
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
	if (!p->p_tty)		/* If no tty, try tty of parent's parent */
		p->p_tty = udata.u_ptab->p_pptr->p_tty;
	p->p_uid = udata.u_ptab->p_uid;
	udata.u_ptab = p;

	memset(&udata.u_utime, 0, 4 * sizeof(clock_t));	/* Clear tick counters */

	rdtime32(&udata.u_time);
	if (udata.u_cwd)
		i_ref(udata.u_cwd);
	if (udata.u_root)
		i_ref(udata.u_root);
	udata.u_cursig = 0;
	udata.u_error = 0;

	/* Set default priority */
	p->p_priority = MAXTICKS;

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
	for (p = ptab; p < ptab + maxproc; p++)
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
			for (p = ptab; p < ptab + maxproc; p++)
				if (p->p_status != P_EMPTY
				    && p->p_pid == nextpid)
					newp->p_pid = 0;	/* try again */
		}
		if (pagemap_alloc(newp) == 0) {
			newp->p_status = P_FORKING;
			nproc++;
		} else {
			udata.u_error = ENOMEM;
			newp = NULL;
                }
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
	uint8_t nr;

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
				((unsigned long)nr) << 16)) >> 8;
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
	/* Increment processes and global tick counters */
	if (udata.u_ptab->p_status == P_RUNNING) {
		if (udata.u_insys)
			udata.u_stime++;
		else
			udata.u_utime++;
	}

	/* Do once-per-decisecond things - this doesn't work out well on
	   boxes with 64 ticks/second.. need a better approach */
	if (++ticks_this_dsecond == TICKSPERSEC / 10) {
		static ptptr p;

		/* Update global time counters */
		ticks_this_dsecond = 0;
		ticks.full++;

		/* Update process alarm clocks and timeouts */
		for (p = ptab; p < ptab + maxproc; ++p) {
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
	}
#ifndef CONFIG_SINGLETASK
	/* Check run time of current process */
	if ((++runticks >= udata.u_ptab->p_priority)
	    && !udata.u_insys && inint && nready > 1) {	/* Time to switch out? */
#ifdef DEBUG
		kputs("[preempt]");
		kprintf("Prio = %d\n", udata.u_ptab->p_priority);
#endif
		udata.u_insys = true;
		udata.u_ptab->p_status = P_READY;
		switchout();
		udata.u_insys = false;	/* We have switched back in */
	}
#endif
}

#ifdef DEBUG
#include "syscall_name.h"
#endif

extern int16_t kernel_flag;	/* true when in a syscall etc, maintained by the
				   asm interfaces but visible in C */

// Fuzix system call handler
// we arrive here from syscall.s with the kernel paged in, using the kernel stack, interrupts enabled.
void unix_syscall(void)
{
	// NO LOCAL VARIABLES PLEASE
	udata.u_insys = true;
	udata.u_error = 0;

	/* Fuzix saves the Stack Pointer and arguments in the
	 * Assembly Language Function handler in lowlevel.s
	 */
	if (udata.u_callno >= FUZIX_SYSCALL_COUNT) {
		udata.u_error = EINVAL;
	} else {
#ifdef DEBUG
		kprintf("\t\tpid %d: syscall %d\t%s(%x, %x, %x)\n",
			udata.u_ptab->p_pid, udata.u_callno,
			syscall_name[udata.u_callno], udata.u_argn,
			udata.u_argn1, udata.u_argn2);
#endif
		// dispatch system call
		udata.u_retval = (*syscall_dispatch[udata.u_callno]) ();

#ifdef DEBUG
		kprintf("\t\t\tpid %d: ret syscall %d, ret %x err %d\n",
			udata.u_ptab->p_pid, udata.u_callno,
			udata.u_retval, udata.u_error);
#endif
	}
	udata.u_ptab->p_timeout = 0;
	chksigs();

	di();
	if (runticks >= udata.u_ptab->p_priority && nready > 1) {
		/* Time to switch out? - we may have overstayed our welcome inside
		   a syscall so switch straight afterwards */
		udata.u_ptab->p_status = P_READY;
		switchout();
	}
	ei();
	udata.u_insys = false;
}

void sgrpsig(uint16_t pgrp, uint16_t sig)
{
	ptptr p;
	for (p = ptab; p < ptab + maxproc; ++p) {
		if (-p->p_pgrp == pgrp)
			ssig(p, sig);
	}
}

/* This sees if the current process has any signals set, and handles them.
 */
void chksigs(void)
{
	uint8_t j;

	// any signals pending?
	if (!(udata.u_ptab->p_pending & ~udata.u_ptab->p_held)) {
		return;
	}
	// dispatch the lowest numbered signal
	for (j = 1; j < NSIGS; ++j) {
		if (!
		    (sigmask(j) & udata.u_ptab->p_pending & ~udata.u_ptab->
		     p_held))
			continue;
		/* This is more complex than in V7 - we have multiple
		   behaviours (plus the unimplemented as yet core dump) */
		if (udata.u_sigvec[j] == SIG_DFL) {
			/* SIGCONT is subtle - we woke the process to handle
			   the signal so ignoring here works fine */
			if (j == SIGCHLD || j == SIGURG ||
			    j == SIGIO || j == SIGCONT)
				continue;
			/* FIXME: core dump on some signals */
#ifdef DEBUG
			kputs("process terminated by signal: ");
#endif
			doexit(0, j);
		}

		if (udata.u_sigvec[j] != SIG_IGN) {
			/* Arrange to call the user routine at return */
			udata.u_ptab->p_pending &= ~sigmask(j);	// unset the bit
#ifdef DEBUG
			kprintf("about to process signal %d\n", j);
#endif
			udata.u_cursig = j;
			break;
		}
	}
}

/*
 *	Send signal, avoid touching uarea
 */
void ssig(ptptr proc, uint16_t sig)
{
	uint16_t sigm;
	irqflags_t irq;

	sigm = sigmask(sig);

	irq = di();
	/* FIXME:
	   SIGCONT needs to wake even if ignored.
	   SIGSTOP needs to actually stop the task once it hits
	   the syscall exit path */
	if (proc->p_status != P_EMPTY) {	/* Presumably was killed just now */
		if (sig == SIGCONT || !(proc->p_ignored & sigm)) {
			/* Don't wake for held signals */
			if (!(proc->p_held & sigm)) {
				switch (proc->p_status) {
				case P_PAUSE:
				case P_WAIT:
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

void doexit(int16_t val, int16_t val2)
{
	int16_t j;
	ptptr p;
	irqflags_t irq;

#ifdef DEBUG
	kprintf("process %d exiting\n", udata.u_ptab->p_pid);

	kprintf
	    ("udata.u_page %u, udata.u_ptab %x, udata.u_ptab->p_page %u\n",
	     udata.u_page, udata.u_ptab, udata.u_ptab->p_page);
#endif
	if (udata.u_ptab->p_pid == 1)
		panic("killed init");

	_sync();		/* Not necessary, but a good idea. */

	irq = di();

	/* Discard our memory before we blow away and reuse the memory */
	pagemap_free(udata.u_ptab);

	for (j = 0; j < UFTSIZE; ++j) {
		if (udata.u_files[j] != NO_FILE)
			doclose(j);
	}


	udata.u_ptab->p_exitval = (val << 8) | (val2 & 0xff);

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

	/* See if we have any children. Set child's parents to our parent */
	for (p = ptab; p < ptab + maxproc; ++p) {
		if (p->p_status && p->p_pptr == udata.u_ptab
		    && p != udata.u_ptab)
			p->p_pptr = udata.u_ptab->p_pptr;
	}
	irqrestore(irq);
#ifdef DEBUG
	kprintf
	    ("udata.u_page %u, udata.u_ptab %x, udata.u_ptab->p_page %u\n",
	     udata.u_page, udata.u_ptab, udata.u_ptab->p_page);
#endif
#ifdef CONFIG_ACCT
	acctexit(p);
#endif
	/* FIXME: send SIGCLD here */
	/* FIXME: POSIX.1 says that SIG_IGN for SIGCLD means don't go
	   zombie, just clean up as we go */
	/* Wake up a waiting parent, if any. */
	wakeup((char *) udata.u_ptab->p_pptr);

	udata.u_ptab->p_status = P_ZOMBIE;
	nready--;
	nproc--;

	switchin(getproc());
	panic("doexit: won't exit");
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
	_execve();
	panic("no /init");	/* BIG Trouble if we Get Here!! */
}
