/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied and taught about using the passwd file listed shell for FUZIX */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

struct	group	*grp;
struct	passwd	*pwd;

void done(const char *shell)
{
	register int i;
	const char *sp = strrchr(shell, '/');
	
	if (sp)
		sp++;
	else
		sp = shell;

	setuid(getuid());
	for (i=3; i<15; i++)
		close(i);
	execl(shell, sp, 0);
	printf("No shell!\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	register int i;

	if((pwd=getpwuid(getuid())) == NULL) {
		printf("You do not exist!\n");
		done("/bin/sh");
	}

	if(argc != 2) {
		printf("usage: newgrp groupname\n");
		done(pwd->pw_shell);
	}
	if((grp=getgrnam(argv[1])) == NULL) {
		printf("%s: no such group\n", argv[1]);
		done(pwd->pw_shell);
	}
	for(i=0;grp->gr_mem[i];i++) 
		if(strcmp(grp->gr_mem[i], pwd->pw_name) == 0)
			break;
	if(grp->gr_mem[i] == 0 && strcmp(grp->gr_name,"other")) {
		printf("Sorry\n");
		done(pwd->pw_shell);
	}

	if(grp->gr_passwd[0] != '\0' && pwd->pw_passwd[0] == '\0') {
		if(strcmp(grp->gr_passwd, crypt(getpass("Password:"),grp->gr_passwd)) != 0) {
			printf("Sorry\n");
			done(pwd->pw_shell);
		}
	}
	if(setgid(grp->gr_gid) < 0)
		perror("setgid");
	done(pwd->pw_shell);
}
