#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/* Will move into arch code: for most platforms is a 'don't care' as
   switching is cheap */
#define is_common(a,b)	(udata.u_base >= (char *) PROGTOP)

/*
 * If System is in context or destination is in Common memory
 * (F000-FFFF is always in context), simply copy the data.
 * Otherwise perform Interbank move (uput) to User bank.
 */

unsigned int uputsys(unsigned char *from, unsigned int size)
{
	if (udata.u_sysio || is_common(from, size))
		memcpy(udata.u_base, from, size);
	else
		uput(from, udata.u_base, size);
	return size;
}

unsigned int ugetsys(unsigned char *to, unsigned int size)
{
	if (udata.u_sysio || is_common(from, size))
		memcpy(to, udata.u_base, size);
	else
		uget(udata.u_base, to, size);
	return size;
}
