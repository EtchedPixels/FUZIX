#ifndef	__GRP_H
#define	__GRP_H

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
struct group
{
  char *gr_name;		/* group name     */
  char *gr_passwd;		/* group password */
  gid_t gr_gid;			/* group ID       */
  char **gr_mem;		/* group members  */
};


extern int  setgrent(void);
extern void endgrent(void);
extern struct group *getgrent(void);

extern int putgrent(struct group *, FILE *);
extern int getgr(gid_t uid, char *buf);

extern struct group *getgrgid(gid_t);
extern struct group *getgrnam(char *);


#endif /* grp.h  */
