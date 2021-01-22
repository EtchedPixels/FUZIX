#include <kernel.h>
#include <version.h>
#include <kdata.h>

arg_t _execve(void)
{
	panic("execve");
}

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

