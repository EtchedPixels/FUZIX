#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tty.h>

#ifdef CONFIG_LEVEL_2

int in_group(uint16_t gid)
{
	uint16_t *g = udata.u_groups;
	uint16_t *p = g + udata.u_ngroup;
	while(g < p)
		if (*g++ == gid)
			return 1;
	return 0;
}

static int orphan_pgrp(uint16_t pgrp, uint16_t sid)
{
	ptptr p;

	for (p = ptab; p < ptab_end; ++p) {
		/* A group is not orphan if a process exists that is
		   - a member of the pgrp
		   - has a parent that is not a member of the pgrp
		   - that parent shares the session of the pgrp

		   or in plain English - there is a process managing the group
		 */
		if (p->p_pgrp == pgrp) {
			if (p->p_pptr->p_pgrp != pgrp &&
				p->p_pptr->p_session == sid)
				return 0;
		}
	}
	return 1;
}

/* Keep all the process group and tty complexity as far out of the tty layer as we can so it has
   no impact on a level 1 host */

static uint8_t jobop(uint8_t minor, uint8_t sig, struct tty *t, uint8_t ign)
{
	struct s_queue *q = &ttyinq[minor];
	uint8_t ignored = 0;
	ptptr p = udata.u_ptab;

	for(;;) {
		/* If it is dead just return ready - the caller will do the EIO handling */
		if ((t->flag & TTYF_DEAD) && (sig != SIGTTIN || !q->q_count))
			return 0;

		if (!t->pgrp || udata.u_ptab->p_pgrp == t->pgrp
		        || udata.u_ptab->p_tty != minor)
			return 0;	/* Proceed */
#ifdef DEBUG
	        kprintf("[stop %d %d %d]\n",
	                t->pgrp, udata.u_ptab->p_pgrp, udata.u_ptab->p_tty);
#endif
		if ((udata.u_ptab->p_sig[1].s_held & sigmask(sig)) || udata.u_sigvec[sig] == SIG_IGN)
			ignored = 1;

		if ((ignored && ign) || orphan_pgrp(p->p_pgrp, p->p_session)) {
			udata.u_error = EIO;
			return 1;
		}
		if (ignored)
			return 0;

		/* Check: self or session ? */
		ssig(udata.u_ptab, sig);
		/* If we have nothing to deliver we will stop here until we get SIGCONT. If we have
		   handlers to run first we will return -1, EINTR so the signal is processed */
		if (chksigs()) {
			udata.u_error = EINTR;
			return 1;
		}
	}
}

/* For input readign with SIGTTIN blocked gets you an EIO */
uint8_t jobcontrol_in(uint8_t minor, struct tty *t)
{
	if (!jobop(minor, SIGTTIN, t, 0))
		return 0;
	if (udata.u_done == 0) {
		udata.u_error = EINTR;
		udata.u_done = (usize_t)-1;
	}
	return 1;
}


/* For output ignored stop is taken to mean as write anyway */
uint8_t jobcontrol_out(uint8_t minor, struct tty *t)
{
	if (!(t->termios.c_lflag & TOSTOP))
		return 0;
        if (!jobop(minor, SIGTTOU, t, 1))
		return 0;
	if (udata.u_done == 0) {
		udata.u_error = EINTR;
		udata.u_done = (usize_t)-1;
	}
	return 1;
}

/* Keep all the ioctl noise in the level2.c code not anywhere shared with L1 */
uint8_t jobcontrol_ioctl(uint8_t minor, struct tty *t, uarg_t request)
{
	switch(request) {
	case TCSETSF:
	case TCSETSW:
	case TCSETS:
	case TIOCFLUSH:
	case TIOCHANGUP:
	case TIOCOSTOP:
	case TIOCOSTART:
	case TIOCSWINSZ:
	case TIOCSPGRP:
		/* This should be safe .. but if you spin here bug me 8) */
		return jobop(minor, SIGTTOU, t, 1);
	default:
		return 0;
	}
}

int tcsetpgrp(struct tty *t, char *data)	/* data is user pointer */
{
	uint16_t grp = ugeti(data);
	uint16_t ses = udata.u_ptab->p_session;
        ptptr p;

	/* Controlling tty check is done by caller */

	/* No change -> ok */
	if (grp == t->pgrp)
		return 0;

	for (p = ptab; p < ptab_end; ++p) {
		/* The group exists */
		if (p->p_pgrp == grp) {
			/* but not within our session */
			if (p->p_session != ses && !super())
				return -1;
			/* So it's a valid group and in our session */
			t->pgrp = grp;
			return 0;
		}
	}
	udata.u_error = EINVAL;
	return -1;
}

#endif
