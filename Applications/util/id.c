/* id - return uid and gid		Author: John J. Marco */

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/* 		----- id.c -----					*/
/* Id - get real and effective user id and group id			*/
/* Author: John J. Marco						*/
/*	   pa1343@sdcc15.ucsd.edu					*/
/* Modified for UZI180 by H. Peraza                                     */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>


int main(int argc, char *argv[])
{
    register struct passwd *pwd;
    register struct group *grp;
    int uid, gid, euid, egid;

    uid = getuid();
    gid = getgid();
    euid = geteuid();
    egid = getegid();

    if ((pwd = getpwuid(uid)) == NULL)
	printf("uid=%d ", uid);
    else
	printf("uid=%d(%s) " , uid, pwd->pw_name);

    if ((grp = getgrgid(gid)) == NULL)
	printf("gid=%d ", gid);
    else
	printf("gid=%d(%s) ", gid, grp->gr_name);

    if (uid != euid) {
	if ((pwd = getpwuid(euid)) != NULL)
	    printf("euid=%d(%s) ", euid, pwd->pw_name);
	else
	    printf("euid=%d ", euid);
    }

    if (gid != egid) {
	if ((grp = getgrgid(egid)) != NULL)
	    printf("egid=%d(%s) ", egid, grp->gr_name);
	else
	    printf("egid=%d ", egid);
    }

    printf("\n");
    return 0;
}
