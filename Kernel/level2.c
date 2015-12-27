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
