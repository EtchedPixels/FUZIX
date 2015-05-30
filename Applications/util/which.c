#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int  quit, found;
    char *envpath;
    char *path, *cp;
    static char buf[512];
    static char patbuf[512];

    if (argc < 2) {
	fprintf(stderr, "Usage: which cmd [cmd, ..]\n");
	return 1;
    }
    if ((envpath = getenv("PATH")) == 0) {
	envpath = ".";
    }
    argv[argc] = 0;
    for (argv++; *argv; argv++) {

	strcpy(patbuf, envpath);
	cp = path = patbuf;
	quit = found = 0;

	while (!quit) {
	    cp = index(path, ':');
	    if (cp == NULL) {
		quit++;
	    } else {
		*cp = '\0';
	    }
	    snprintf(buf, 512, "%s/%s", (*path ? path : "."), *argv);
	    path = ++cp;

	    if (access(buf, 1) == 0) {
		printf("%s\n", buf);
		found++;
	    }
	}
	if (!found) {
	    printf("No %s in %s\n", *argv, envpath);
	}
    }

    return 0;
}
