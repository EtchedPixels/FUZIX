#include <stdio.h>
#include <unistd.h>
#include <utime.h>

static time_t settime;

int main(int argc, const char *argv[])
{
    int er = 0, i, ncreate = 0;
    struct stat sbuf;
    struct utimbuf tbuf;

    if ((argv[1][0] == '-') && (argv[1][1] == 'c'))
	ncreate = 1;

    for (i = ncreate + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (stat(argv[i], &sbuf)) {
		if (!ncreate)
		    er = close(creat(argv[i], 0666));
	    } else {
	        settime = time(NULL);
                tbuf.actime = settime;
	        tbuf.modtime = settime;
                er |= utime(argv[i], &tbuf);
	    }
	}
    }
    return er;
}
