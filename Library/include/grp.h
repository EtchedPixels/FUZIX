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

extern void setgrent(void);
extern void endgrent(void);
extern struct group *getgrent(void);

extern struct group *getgrgid(const gid_t __gid);
extern struct group *getgrnam(const char *__name);

extern struct group *fgetgrent(FILE *__file);

extern int initgroups(const char *__user, gid_t __gid);

extern struct group * __getgrent(int __grp_fd);

extern char *_path_group;

#endif /* _GRP_H */
