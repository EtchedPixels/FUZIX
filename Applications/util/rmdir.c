#include <unistd.h>
#include <stdio.h>
#include <string.h>

unsigned short newmode;

int remove_dir(char *name, int f)
{
    int er, era = 2;
    char *line;
    char *p = name + strlen(name) - 1;
    while(p >= name && *p == '/')
        *p-- = '\0';

    while (((er = rmdir(name)) == 0)
	   && ((line = rindex(name, '/')) != NULL) && f) {
	while ((line > name) && (*line == '/')) --line;
	line[1] = 0;
	era = 0;
    }
    return (er && era);
}


int main(int argc, char **argv)
{
    int i, parent = 0, er = 0;

    if ((argv[1][0] == '-') && (argv[1][1] == 'p'))
	parent = 1;

    newmode = 0666 & ~umask(0);

    for (i = parent + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (remove_dir(argv[i], parent)) {
	        fprintf(stderr, "rmdir: cannot remove directory %s\n", argv[i]);
		er = 1;
	    }
	} else {
	    fprintf(stderr, "rmdir: usage error\n");
	    exit(1);
	}
    }
    return er;
}

