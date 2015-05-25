/* execv.c
 *
 * function(s)
 *	  execv - load and execute a program
 */  
    
#include <unistd.h>
#include <paths.h>

int execv(const char *pathP, char * const argv[]) 
{
	return execve(pathP, argv, (void *)environ);
}


