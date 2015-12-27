#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>

/*******************************************
  setgroups (ngroup, groups)     Function ??
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
		return -1
	}
	if (ngroup && uget(groups, udata.u_groups, ngroup * sizeof(uint16_t)))
		return -1;
	udata.u_ngroup = ngroup;
	return 0;
}

#undef ngroup
#undef groups

/*******************************************
  getgroups (ngroup, groups)     Function ??
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
		return;
	}
	if (uput(groups, udata.u_groups, ngroup * sizeof(uint16_t)) < 0)
		return -1;
	return udata.u_ngroup;
}

#undef ngroup
#undef groups

/*******************************************
  getrlimit (res, rlimit)        Function ??
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
  setrlimit (res, rlimit)        Function ??
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

	o = uata.u_rlimit + res;
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

