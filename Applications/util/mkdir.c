#include <stdio.h>
#include <string.h>
#include <unistd.h>

unsigned short newmode;

void writes(const char *p)
{
    write(2, p, strlen(p));
}

int make_dir(const char *name, int f)
{
    char *line;
    static char iname[512];

    int l = strlen(name) - 1;

    /* FIXME: Size check ! */
    strlcpy(iname, name, 512);
    if (l && iname[l] == '/')
        iname[l] = 0;
    if (((line = strrchr(iname, '/')) != NULL) && f) {
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
	        writes("mkdir: cannot create directory '");
	        writes(argv[i]);
	        perror("'");
		er = 1;
	    }
	} else {
	    writes("mkdir: usage error\n");
	    exit(1);
	}
    }
    return er;
}
