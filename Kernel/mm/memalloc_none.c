#include <kernel.h>
#include <kdata.h>

/*
 *	Memory allocator system calls for systems that have no
 *	malloc level functionality. Link this into the platforms as
 *	needed so we can get rid of the ugly ifdef and dependnecy mess
 *	we currently have.
 */

arg_t _memalloc(void)
{
	udata.u_error = ENOMEM;
	return -1;
}

arg_t _memfree(void)
{
	udata.u_error = ENOMEM;
	return -1;
}

