/* execvp.c
 *
 * function(s)
 *	  execvp - load and execute a program
 */  
    
#include <unistd.h>
#include <paths.h>

int execvp(const char *pathP, char *const argv[]) 
{
	return execve(_findPath(pathP), argv, (void *)environ);
}
