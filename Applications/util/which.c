#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int quit, found;
	char *envpath;
	char *path, *cp;
	static char buf[512];
	static char patbuf[512];

	if (argc < 2) {
		fprintf(stderr, "Usage: which cmd [cmd, ..]\n");
		return 1;
	}
	if ((envpath = getenv("PATH")) == NULL) {
		envpath = "/bin";
	}
	argv[argc] = NULL;
	found = 0;
	argc--;
	for (argv++; *argv; argv++) {

		snprintf(patbuf, sizeof(patbuf), "%s", envpath);
		cp = path = patbuf;
		quit = 0;

		while (!quit) {
			cp = strchr(path, ':');
			if (cp == NULL) {
				quit++;
			} else {
				*cp = 0;
			}
			snprintf(buf, sizeof(buf), "%s/%s", (*path ? path : "."), *argv);
			path = ++cp;

			if (access(buf, X_OK) == 0) {
				printf("%s\n", buf);
				found++;
				break;
			}
		}
	}

	/*
	 *  0: if all the commands have been found and are executable
	 *  1: if at least one command is not found or not executable
	 */
	return found != argc;
}
