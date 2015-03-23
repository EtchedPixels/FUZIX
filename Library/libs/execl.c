/* execl.c
 *
 * function(s)
 *	  execl - load and run a program
 */  
    
#include <unistd.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
    
/* Find file in pathes:
 * 1. /name or ./name or ../name is already qualified names
 * 2. else search in all pathes described in env var PATH (if this
 *    var is not exist, _PATH_DEFPATH is used)
 * 3. else search in current directory
 * 4. else return NULL (execve() interpretes NULL as non existent file!)
 */ 
const char *_findPath(const char *path) 
{
	char *p;
	const char *envp;
	static char name[PATHLEN + 1];
	if (*path == '/' || /* qualified name */ *path == '.')
		return path;

	/* search for pathes list */ 
	if ((envp = getenv("PATH")) == NULL)
		envp = _PATH_DEFPATH;

	/* lookup all pathes */ 
	while (*envp) {
		p = name;
		while (*envp && (*p = *envp++) != ':') {
			if ((uint) (p - name) >= sizeof(name))
				break;
			++p;
		}
		if (*--p != '/')
			*++p = '/';
		++p;
		if ((p - name) + strlen(path) >= sizeof(name))
			break;
		strcpy(p, path);
		if (access(name, 0) == 0)
			return name;
	}
	if (access(path, 0) == 0)	/* file exist in current dir */
		return name;
	return NULL;
}

#ifndef __CC65__

/* FIXME: The 6502 calling sequence means these need a different implementation */
int execl(const char *pathP, const char *arg0, ...) 
{
	return execve(pathP, &arg0, environ);
}

int execlp(const char *pathP, const char *arg0, ...) 
{
	return execve(_findPath(pathP), &arg0, environ);
}
#else
int execl(const char *pathP, const char *arg0, ...) 
{
	/* BUG: this should be ENOSYS, but Fuzix doesn't have that yet */
	errno = EINVAL;
	return -1;
}

int execlp(const char *pathP, const char *arg0, ...) 
{
	/* BUG: this should be ENOSYS, but Fuzix doesn't have that yet */
	errno = EINVAL;
	return -1;
}
#endif
