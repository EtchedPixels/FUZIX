/*
 *	vfork is .. fork at least for now. Lay the groundwork
 *
 *	And yes this is compliant wtih POSIX.
 */

#include <unistd.h>

pid_t vfork(void)
{
    return fork();
}
