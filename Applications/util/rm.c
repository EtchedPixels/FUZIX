#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

char *basename(char *name)
{
    char *base = rindex(name, '/');
    return base ? base + 1 : name;
}


int main(int argc, const char *argv[])
{
    int i /*, recurse = 0, interact =0 */ ;
    struct stat sbuf;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (!lstat(argv[i], &sbuf)) {
		if (unlink(argv[i])) {
		    fprintf(stderr, "rm: could not remove %s\n", argv[i]);
		}
	    }
	}
    }
    return 0;
}
