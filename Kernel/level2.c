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

static void jobop(uint8_t minor, uint8_t sig, struct tty *t)
{
	if (!t->pgrp || udata.u_ptab->p_pgrp == t->pgrp
	        || udata.u_ptab->p_tty != minor)
		return;
#ifdef DEBUG
        kprintf("[stop %d %d %d]\n",
                t->pgrp, udata.u_ptab->p_pgrp, udata.u_ptab->p_tty);
#endif
	ssig(udata.u_ptab, sig);
	/* So we halt */
	psleep(0);
}

void jobcontrol_in(uint8_t minor, struct tty *t)
{
        jobop(minor, SIGTTIN, t);
}


void jobcontrol_out(uint8_t minor, struct tty *t)
{
	if (!(t->termios.c_lflag & TOSTOP))
		return;
        jobop(minor, SIGTTOU, t);
}

int tcsetpgrp(struct tty *t, char *data)	/* data is user pointer */
{
	uint16_t grp = ugetw(data);
	uint16_t ses = udata.u_ptab->p_session;
        ptptr p;
	uint8_t found = 0;

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
