#include <stdio.h>
#include <unistd.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

static time_t settime;

int main(int argc, const char *argv[])
{
    int er = 0, res, i, ncreate = 0, fd;
    struct stat sbuf;
    struct utimbuf tbuf;

    if ((argv[1][0] == '-') && (argv[1][1] == 'c')) {
	ncreate = 1;
    }

    for (i = ncreate + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (stat(argv[i], &sbuf) < 0) {
		if (!ncreate) {
		    fd = creat(argv[i], 0666);
		    if (fd < 0) {
			res = -1;
		    } else {
			close(fd);
			res = 0;
		    }
		} else {
		    /* -c: Quietly ignore missing files */
		    res = 0;
		}
	    } else {
	        settime = time(NULL);
                tbuf.actime = settime;
	        tbuf.modtime = settime;
                res = utime(argv[i], &tbuf);
	    }

	    /* Complain on each failed touch */
	    if (res) {
		fprintf(stderr, "%s: cannot touch '%s': %s\n",
		    argv[0], argv[i], strerror(errno));
	    }

	    /* Accumulate errors */
	    er |= res;
	}
    }
    return (er ? 1 : 0);
}
