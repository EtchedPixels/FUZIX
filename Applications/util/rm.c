#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/* FIXME: need -r -v -i etc */
static void writes(int fd, const char *p)
{
    write(fd, p, strlen(p));
}

char *basename(char *name)
{
    char *base = rindex(name, '/');
    return base ? base + 1 : name;
}


int main(int argc, const char *argv[])
{
    int i /*, recurse = 0, interact =0 */ ;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (unlink(argv[i])) {
	        writes(2, argv[0]);
	        writes(2, ": cannot remove '");
	        writes(2, argv[i]);
	        perror("'");
	    }
	}
    }
    return 0;
}
