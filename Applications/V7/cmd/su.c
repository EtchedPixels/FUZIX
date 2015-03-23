
/* ANSIfield for FUZIX, some security holes fixed */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <pwd.h>

struct passwd *pwd;

/* FIXME: su - and other semantics of later su versions */

int main(int argc, const char *argv[])
{
	register char **p;
	const char *nptr;
	char *password;
	int badsw = 0;
	const char *shell = "/bin/sh";

	/* If fd 0,1,2 are not all open something funny is up so
	   exit */
	if (dup(0) < 3)
		_exit(1);

	if (argc > 1)
		nptr = argv[1];
	else
		nptr = "root";
	if ((pwd = getpwnam(nptr)) == NULL) {
		printf("Unknown id: %s\n", nptr);
		exit(1);
	}
	if (pwd->pw_passwd[0] == '\0' || getuid() == 0)
		goto ok;
	password = getpass("Password:");
	if (badsw
	    || (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) !=
		0)) {
		printf("Sorry\n");
		exit(2);
	}

      ok:
	endpwent();
	if (setgid(pwd->pw_gid) < 0)
		err(1, "setgid");
	if (setuid(pwd->pw_uid) < 0)
		err(1, "setuid");
	if (pwd->pw_shell && *pwd->pw_shell)
		shell = pwd->pw_shell;
	for (p = environ; *p; p++) {
		if (strncmp("PS1=", *p, 4) == 0) {
			*p = "PS1=# ";
			break;
		}
	}
	execl(shell, "su", 0);
	printf("No shell\n");
	exit(3);
}
