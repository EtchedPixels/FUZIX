#ifndef __PWD_H
#define __PWD_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stdio.h>

/* The passwd structure.  */
struct passwd {
	char	*pw_name;	/* Username */
	char	*pw_passwd;	/* Password */
	uid_t	pw_uid; 	/* User ID */
	gid_t	pw_gid; 	/* Group ID */
	char	*pw_gecos;	/* Real name */
	char	*pw_dir;	/* Home directory */
	char	*pw_shell;	/* Shell program */
};

extern void setpwent __P((void));
extern void endpwent __P((void));
extern struct passwd *getpwent __P((void));

extern int putpwent __P((const struct passwd * __p, FILE * __f));
extern int getpw __P((uid_t uid, char *buf));

extern struct passwd *fgetpwent __P((FILE * file));

extern struct passwd *getpwuid __P((uid_t __uid));
extern struct passwd *getpwnam __P((const char *));

extern struct passwd * __getpwent __P((int passwd_fd));

extern char *_path_passwd;

#define getlogin()	getpwnam(getuid())

#endif /* pwd.h  */
