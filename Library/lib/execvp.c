/* execvp.c
 *
 * function(s)
 *	  execvp - load and execute a program
 */  
    
#include <unistd.h>
#include <paths.h>

int execvp(char *pathP, char *argv[]) 
{
	return execve(_findPath(pathP), argv, environ);
}
