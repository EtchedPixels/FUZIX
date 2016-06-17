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

extern void setpwent(void);
extern void endpwent(void);
extern struct passwd *getpwent(void);

extern int putpwent(const struct passwd *__p, FILE *__f);
extern int getpw(uid_t uid, char *buf);

extern struct passwd *fgetpwent(FILE *__file);

extern struct passwd *getpwuid(uid_t __uid);
extern struct passwd *getpwnam(const char *__name);

extern struct passwd * __getpwent(int __passwd_fd);

extern char *_path_passwd;

#endif /* pwd.h  */
