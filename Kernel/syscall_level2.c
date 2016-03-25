#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_LEVEL_2

/*******************************************
  setgroups (ngroup, groups)     Function 73
  int ngroup;
  const uint16_t *groups;
 *******************************************/
#define ngroup (int)udata.u_argn
#define groups (uint16_t *)udata.u_argn1

arg_t _setgroups(void)
{
	if (esuper())
		return -1;
	if (ngroup < 0 || ngroup > NGROUP) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (ngroup && uget(groups, udata.u_groups, ngroup * sizeof(uint16_t)))
		return -1;
	udata.u_ngroup = ngroup;
	return 0;
}

#undef ngroup
#undef groups

/*******************************************
  getgroups (ngroup, groups)     Function 74
  int ngroup;
  uint16_t *groups;
 *******************************************/
#define ngroup (int)udata.u_argn
#define groups (uint16_t *)udata.u_argn1

arg_t _getgroups(void)
{
	if (groups == 0)
		return udata.u_ngroup;
	if (ngroup < udata.u_ngroup) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (uput(udata.u_groups, groups, udata.u_ngroup * sizeof(uint16_t)) < 0)
		return -1;
	return udata.u_ngroup;
}

#undef ngroup
#undef groups

/*******************************************
  getrlimit (res, rlimit)        Function 75
  int res;
  struct rlimit *rlim;
 *******************************************/

#define res (int)udata.u_argn
#define rlim (struct rlimit *)udata.u_argn1

arg_t _getrlimit(void)
{
	if (res < 0 || res >= NRLIMIT) {
		udata.u_error = EINVAL;
		return -1;
	}
	return uput(rlim, udata.u_rlimit + res, sizeof(struct rlimit));
}

#undef res
#undef rlim

/*******************************************
  setrlimit (res, rlimit)        Function 76
  int res;
  const struct rlimit *rlim;
 *******************************************/

#define res (int)udata.u_argn
#define rlim (struct rlimit *)udata.u_argn1

arg_t _setrlimit(void)
{
	staticfast struct rlimit r;
	struct rlimit *o;

	if (res < 0 || res >= NRLIMIT)
		goto bad;

	if (uget(rlim, &r, sizeof(struct rlimit)))
		return -1;

	o = udata.u_rlimit + res;
	if (r.rlim_cur > r.rlim_max)
		goto bad;

	/* Securit check */
	if ((r.rlim_cur > o->rlim_max || r.rlim_max > o->rlim_max) && esuper())
		return -1;
	o->rlim_cur = r.rlim_cur;
	o->rlim_max = r.rlim_max;
	return 0;
bad:	
	udata.u_error = EINVAL;
	return -1;
}

#undef res
#undef rlim

/*******************************************
  setpgid (pid, pgid)        Function 77
  uint16_t npid;
  uint16_t npgid;
 *******************************************/

#define npid (uint16_t)udata.u_argn
#define npgid (uint16_t)udata.u_argn1

arg_t _setpgid(void)
{
	ptptr p, t = NULL;
	uint16_t ses = udata.u_ptab->p_session;
	uint16_t n = npgid;

	/* pid 0 means "self" */
	if (npid == 0)
		t = udata.u_ptab;
	else {
		for (p = ptab; p < ptab_end; ++p) {
			if (p->p_pid == npid)
				t = p;
		}
	}
	if (t == NULL) {
		udata.u_error = ESRCH;
		return -1;
	}

	if (t->p_session != udata.u_ptab->p_session)
		goto invalid;

	if (n == 0)
		n = t->p_pid;

	/* Check if the group is in use with a different session */
	for (p = ptab; p < ptab_end; ++p)
		if (p->p_pgrp == n && p->p_session != ses)
			goto invalid;

	t->p_pgrp = n;
	return 0;

invalid:
	udata.u_error = EPERM;
	return -1;
}

#undef npid
#undef npgid

/*******************************************
  setsid (void)                  Function 78
 *******************************************/

arg_t _setsid(void)
{
	ptptr p;
	uint16_t pid = udata.u_ptab->p_pid;

	for (p = ptab; p < ptab_end; ++p) {
		if (p->p_pgrp == pid || p->p_session == pid) {
			udata.u_error = EPERM;
			return -1;
		}
	}
	p = udata.u_ptab;
	p->p_pgrp = pid;
	p->p_session = pid;
	p->p_tty = 0;
	udata.u_ctty = NULL;
	return 0;
}

/*******************************************
  getsid (pid)                   Function 79
  uint16_t pid;
 *******************************************/

#define pid (uint16_t)udata.u_argn

arg_t _getsid(void)
{
	ptptr p;
	for (p = ptab; p < ptab_end; ++p) {
		if (p->p_pid == pid)
			return p->p_session;
	}
	udata.u_error = ESRCH;
	return -1;
}
#undef pid

#endif
