/* execv.c
 *
 * function(s)
 *	  execv - load and execute a program
 */  
    
#include <unistd.h>
#include <paths.h>

int execv(char *pathP, char *argv[]) 
{
	return execve(pathP, argv, environ);
}


