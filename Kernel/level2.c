#include <kernel.h>
#include <kdata.h>
#include <printf.h>

int in_group(uint16_t gid)
{
	uint16_t *g = udata.u_group;
	uint16_t *p = g + udata.u_ngroup;
	while(g < p)
		if (*g++ == gid)
			return 1;
	return 0;
}

void jobcontrol_in(struct tty *t)
{
	if (udata.u_proc->p_pgrp == t->pgrp)
		return;
	ssig(udata.u_proc, SIGTTIN);
	/* So we halt */
	pause(0);
}

void jobcontrol_out(struct tty *t)
{
	if (udata.u_proc->p_pgrp == t->pgrp)
		return;
	if (!(t->termios.t_lflag & TOSTOP))
		return;
	ssig(udata.u_proc, SIGTTOU);
	/* So we halt */
	pause(0);
}
