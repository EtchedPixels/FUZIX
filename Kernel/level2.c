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

void jobcontrol_in(struct tty *t)
{
	if (udata.u_ptab->p_pgrp == t->pgrp)
		return;
	/* We probably want to special case a helper here because we need
	   to handle the funnier side effects ? */
	ssig(udata.u_ptab, SIGTTIN);
	/* So we halt */
	psleep(0);
}

void jobcontrol_out(struct tty *t)
{
	if (udata.u_ptab->p_pgrp == t->pgrp)
		return;
	if (!(t->termios.c_lflag & TOSTOP))
		return;
	ssig(udata.u_ptab, SIGTTOU);
	/* So we halt */
	psleep(0);
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
			if (p->p_session != ses) {
				udata.u_error = EPERM;
				return -1;
			}
			/* So it's a valid group and in our session */
			t->pgrp = grp;
			return 0;
		}
	}
	udata.u_error = EINVAL;
	return -1;
}

#endif
