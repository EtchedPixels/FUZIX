#ifndef	__PWD_H
#define	__PWD_H

#include <stdio.h>

#ifndef _UID_T
#define _UID_T
typedef int uid_t;
#endif

#ifndef _GID_T
#define _GID_T
typedef int gid_t;
#endif

/* The passwd structure */
struct passwd
{
  char *pw_name;		/* Username       */
  char *pw_passwd;		/* Password       */
  uid_t pw_uid;			/* User ID        */
  gid_t pw_gid;			/* Group ID       */
  char *pw_gecos;		/* Real name      */
  char *pw_dir;			/* Home directory */
  char *pw_shell;		/* Shell program  */
};


extern int  setpwent(void);
extern void endpwent(void);
extern struct passwd *getpwent(void);

extern int putpwent(struct passwd *, FILE *);
extern int getpw(uid_t uid, char *buf);

extern struct passwd *getpwuid(uid_t);
extern struct passwd *getpwnam(char *);


#endif /* pwd.h  */
