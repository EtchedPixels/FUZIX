/* execvp.c
 *
 * function(s)
 *	  execvp - load and execute a program
 */  
    
#include <unistd.h>
#include <stdio.h>
#include <paths.h>

int execvp(const char *pathP, char *const argv[]) 
{
#ifdef PREFER_STACK
	char name[PATHLEN + 1];
#else
	static char name[PATHLEN + 1];
#endif
	return execve(_findPath(name, pathP), argv, (void *)environ);
}
