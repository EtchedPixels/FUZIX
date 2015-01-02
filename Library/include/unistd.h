#ifndef __UNISTD_H
#define __UNISTD_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <syscalls.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

extern long tell __P((int));
extern off_t lseek __P((int, off_t, int));

extern int isatty __P((int));

extern int lstat __P((const char *, struct stat *));
extern int readlink __P((const char *, char *, int));

extern unsigned int sleep __P((unsigned int seconds));

extern char **environ;

extern char * _findPath __P((char *pathname));
extern int execl __P((char *pathname, char *arg0, ...));
extern int execle __P((char *pathname, char *arg0, ...));
extern int execlp __P((char *pathname, char *arg0, ...));
extern int execlpe __P((char *pathname, char *arg0, ...));
extern int execv __P((char *pathname, char *argv[]));
extern int exect __P((char *pathname, char *argv[], char *envp[]));
extern int execvp __P((char *pathname, char *argv[]));
extern int execvpe __P((char *pathname, char *argv[], char *envp[]));

extern char *ttyname __P((int));
extern int system __P((char *));
extern int pause __P((void));
extern int fork __P((void));
extern char *getcwd __P((char *, int));

extern long sysconf __P((int name));
extern long fpathconf __P((int fd, int name));
extern long pathconf __P((const char *path, int name));
extern long _pathconf __P((int name));

#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_HOST_NAME_MAX	3
#define _SC_LOGIN_NAME_MAX	4
#define _SC_CLK_TCK		5
#define _SC_OPEN_MAX		6
#define _SC_PAGESIZE		7
#define _SC_RE_DUP_MAX		8
#define _SC_STREAM_MAX		9
#define _SC_SYMLOOP_MAX		10
#define _SC_TTY_NAME_MAX	11
#define _SC_TZNAME_MAX		12
#define _SC_VERSION		13
#define _SC_PHYS_PAGES		14
#define _SC_AVPHYS_PAGES	15
#define _SC_NPROCESSORS_CONF	16
#define _SC_NPROCESSORS_ONLN	17
/* SYS5 isms: TODO */
#define _SC_NGROUPS_MAX		18
#define _SC_JOB_CONTROL		19
#define _SC_SAVED_IDS		20

#define _SC_FUZIX_LOADAVG1	64
#define _SC_FUZIX_LOADAVG5	65
#define _SC_FUZIX_LOADAVG15	66

#define _PC_LINK_MAX		1
#define _PC_MAX_CANON		2
#define _PC_MAX_INPUT		3
#define _PC_NAME_MAX		4
#define _PC_PATH_MAX		5
#define _PC_PIPE_BUF		6
#define _PC_CHOWN_RESTRICTED	7
#define _PC_NO_TRUNC		8
#define _PC_VDISABLE		9

#define _POSIX_LINK_MAX		_pathconf(_PC_LINK_MAX)
#define _POSIX_MAX_CANON	_pathconf(_PC_MAX_CANON)
#define _POSIX_MAX_INPUT	_pathconf(_PC_MAX_INPUT)
#define _POSIX_NAME_MAX		_pathconf(_PC_NAME_MAX)
#define _POSIX_PATH_MAX		_pathconf(_PC_PATH_MAX)
#define _POSIX_PIPE_BUF		_pathconf(_PC_PIPE_BUF)
#define _POSIX_CHOWN_RESTRICTED _pathconf(_PC_CHOWN_RESTRICTED)
#define _POSIX_NO_TRUNC		_pathconf(_PC_NO_TRUNC)

/* POSIX: show that the clock_ API is present */
#define _POSIX_TIMERS
#define _POSIX_MONTONIC_CLOCK

extern int gethostname(char *name, size_t len);
extern int sethostname(const char *name, size_t len);

#ifndef __STDLIB_H
extern void exit __P((int));
#endif

#ifndef R_OK
#define R_OK	4	/* Test for read permission.  */
#define W_OK	2	/* Test for write permission.  */
#define X_OK	1	/* Test for execute permission.  */
#define F_OK	0	/* Test for existence.	*/
#endif

#define F_ULOCK	0
#define F_LOCK	1
#define F_TLOCK	2
#define F_TEST	3

#endif /* __UNISTD_H */
