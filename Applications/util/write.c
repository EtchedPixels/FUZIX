/*
 * write.c
 *
 * Copyright 2000 Alistair Riddoch
 * ajr@ecs.soton.ac.uk
 *
 * This file may be distributed under the terms of the GNU General Public
 * License v2, or at your option any later version.
 */

/*
 * This is a small version of write for use in the ELKS project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <utmp.h>
#include <fcntl.h>
#include <string.h>

#define TTYPREFIX "/dev/"

void usage(char ** argv)
{
	fprintf(stderr, "%s user [ttyname]\n", argv[0]);
	exit(1);
}

int main(int argc, char ** argv)
{
	static char ttyname[12];
	struct passwd * pwdent;
	int ofd = -1;
	static char buf[255];
	int n;

	if ((argc > 3) || (argc < 2)) {
		usage(argv);
	}
	if ((pwdent = getpwnam(argv[1])) == NULL) {
		fprintf(stderr, "%s: No such user %s\n", argv[0], argv[1]);
		exit(1);
	}
	if (argc == 3) {
		struct stat sbuf;

		strncpy(ttyname, TTYPREFIX, 6);
		strncat(ttyname, argv[2], 6);
		ttyname[11] = '\0';
		if (stat(ttyname, &sbuf) != 0) {
			fprintf(stderr, "%s: No such tty %s\n",
				argv[0], ttyname);
			exit(1);
		}
		if (pwdent->pw_uid != sbuf.st_uid) {
			fprintf(stderr, "%s: User %s is not logged onto %s\n",
				argv[0], argv[1], argv[2]);
			exit(1);
		}
		if ((ofd = open(ttyname, O_WRONLY)) < 0) {
			fprintf(stderr, "%s: %s has messages disabled on %s\n",
				argv[0], argv[1], argv[2]);
			exit(1);
		}
	} else { /* argc == 2 */
		struct utmp * utmp;
		setutent();
		while ((utmp = getutent()) != NULL) {
			if ((strcmp(pwdent->pw_name, utmp->ut_user) == 0) &&
			    (utmp->ut_type == USER_PROCESS)) {
				strncpy(ttyname, TTYPREFIX, 6);
				strncat(ttyname, utmp->ut_line, 6);
				ttyname[11] = '\0';
				if ((ofd = open(ttyname, O_WRONLY)) >= 0) {
					break;
				}
			}
		}
		if (ofd < 0) {
			fprintf(stderr, "%s: User %s not logged in, or has messages disabled\n",
				argv[0], argv[1]);
			exit(1);
		}
	}
	if ((pwdent = getpwnam(argv[1])) == NULL) {
		fprintf(stderr, "%s: Who are you?\n", argv[0]);
		exit(1);
	}
	write(ofd, "Message from ", 13);
	write(ofd, pwdent->pw_name, strlen(pwdent->pw_name));
	write(ofd, "\n", 1);
	while ((n = read(STDIN_FILENO, buf, 254)) > 0) {
		buf[254] = '\0';
		write(ofd, buf, n);
	}
	write(ofd, "EOF\n", 4);
	
	exit(0);
}
