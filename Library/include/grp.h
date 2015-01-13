#ifndef __GRP_H
#define __GRP_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stdio.h>

#define GR_MAX_GROUPS	32
#define GR_MAX_MEMBERS	16

/* The group structure */
struct group {
	char	*gr_name;	/* Group name.	*/
	char	*gr_passwd;	/* Password.	*/
	int	gr_gid; 	/* Group ID.	*/
	char	**gr_mem;	/* Member list. */
};

extern void setgrent __P((void));
extern void endgrent __P((void));
extern struct group *getgrent __P((void));

extern struct group *getgrgid __P((const gid_t gid));
extern struct group *getgrnam __P((const char * name));

extern struct group * fgetgrent __P((FILE * file));

extern int setgroups __P((size_t n, int *groups));
extern int initgroups __P((const char * user, gid_t gid));

extern struct group * __getgrent __P((int grp_fd));

extern char *_path_group;

#endif /* _GRP_H */
