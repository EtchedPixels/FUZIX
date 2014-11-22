#include <kernel.h>
#include <kdata.h>
#include <printf.h>

unsigned int uputsys(unsigned char *from, unsigned int size)
{
	if (udata.u_sysio)
		memcpy(udata.u_base, from, size);
	else
		uput(from, udata.u_base, size);
	return size;
}

unsigned int ugetsys(unsigned char *to, unsigned int size)
{
	if (udata.u_sysio)
		memcpy(to, udata.u_base, size);
	else
		uget(udata.u_base, to, size);
	return size;
}
