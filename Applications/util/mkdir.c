#include <stdio.h>
#include <string.h>
#include <unistd.h>

unsigned short newmode;

int make_dir(const char *name, int f)
{
    char *line;
    static char iname[256];

    int l = strlen(name) - 1;

    /* FIXME: Size check ! */
    strcpy(iname, name);
    if (l && iname[l] == '/')
        iname[l] = 0;
    if (((line = rindex(iname, '/')) != NULL) && f) {
	while ((line > iname) && (*line == '/'))
	    --line;
	line[1] = 0;
	make_dir(iname, 1);
    }
    if (mkdir(name, newmode) && !f)
	return (1);
    else
	return (0);
}


int main(int argc, char *argv[])
{
    int i, parent = 0, er = 0;

    if ((argv[1][0] == '-') && (argv[1][1] == 'p'))
	parent = 1;

    newmode = 0777 & ~umask(0);

    for (i = parent + 1; i < argc; i++) {
	if (argv[i][0] != '-') {

	    if (make_dir(argv[i], parent)) {
		fprintf(stderr, "mkdir: cannot create directory %s\n", argv[i]);
		er = 1;
	    }
	} else {
	    fprintf(stderr, "mkdir: usage error\n");
	    exit(1);
	}
    }
    return er;
}
