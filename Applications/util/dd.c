/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * The "dd" built-in command.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>


typedef unsigned char BOOL;

#ifndef FALSE
#define FALSE  0
#define TRUE   1
#endif

#define	PAR_NONE	0
#define	PAR_IF		1
#define	PAR_OF		2
#define	PAR_BS		3
#define	PAR_COUNT	4
#define	PAR_SEEK	5
#define	PAR_SKIP	6


typedef struct {
    char *name;
    int value;
} PARAM;


static PARAM params[] =
{
    {"if",    PAR_IF},
    {"of",    PAR_OF},
    {"bs",    PAR_BS},
    {"count", PAR_COUNT},
    {"seek",  PAR_SEEK},
    {"skip",  PAR_SKIP},
    {NULL,    PAR_NONE}
};


static long getnum(char *);

BOOL intflag;

static char localbuf[8192];

int main(int argc, char *argv[])
{
    char *str;
    char *cp;
    PARAM *par;
    char *infile;
    char *outfile;
    int infd;
    int outfd;
    int incc;
    int outcc;
    int blocksize;
    long count;
    long seekval;
    long skipval;
    long intotal;
    long outtotal;
    long inmax;
    char *buf;
    int ret = 1;

    infile = NULL;
    outfile = NULL;
    seekval = 0;
    skipval = 0;
    blocksize = 512;
    count = 0x7fffffff;
    inmax = 0;

    while (--argc > 0) {
	str = *++argv;
	cp = strchr(str, '=');
	if (cp == NULL) {
	    fprintf(stderr, "Bad dd argument\n");
	    return 1;
	}
	*cp++ = '\0';

	for (par = params; par->name; par++) {
	    if (strcmp(str, par->name) == 0)
		break;
	}

	switch (par->value) {
	case PAR_IF:
	    if (infile) {
		fprintf(stderr, "Multiple input files illegal\n");
		return 1;
	    }
	    infile = cp;
	    break;

	case PAR_OF:
	    if (outfile) {
		fprintf(stderr, "Multiple output files illegal\n");
		return 1;
	    }
	    outfile = cp;
	    break;

	case PAR_BS:
	    blocksize = getnum(cp);
	    if (blocksize <= 0) {
		fprintf(stderr, "Bad block size value\n");
		return 1;
	    }
	    break;

	case PAR_COUNT:
	    count = getnum(cp);
	    if (count < 0) {
		fprintf(stderr, "Bad count value\n");
		return 1;
	    }
	    break;

	case PAR_SEEK:
	    seekval = getnum(cp);
	    if (seekval < 0) {
		fprintf(stderr, "Bad seek value\n");
		return 1;
	    }
	    break;

	case PAR_SKIP:
	    skipval = getnum(cp);
	    if (skipval < 0) {
		fprintf(stderr, "Bad skip value\n");
		return 1;
	    }
	    break;

	default:
	    fprintf(stderr, "Unknown dd parameter\n");
	    return 1;
	}
    }

    buf = localbuf;
    if (blocksize > sizeof(localbuf)) {
	buf = malloc(blocksize);
	if (buf == NULL) {
	    fprintf(stderr, "Cannot allocate buffer\n");
	    return 1;
	}
    }
    intotal = 0;
    outtotal = 0;

    if (infile) {
	infd = open(infile, 0);
	if (infd < 0) {
	    perror(infile);
	    if (buf != localbuf)
	        free(buf);
	    return 1;
	}
    } else
    	infd = 0;

    if (outfile) {
        outfd = creat(outfile, 0666);
        if (outfd < 0) {
            perror(outfile);
	    close(infd);
	    if (buf != localbuf)
	        free(buf);
	    return 1;
	}
    } else {
	outfile = "-";
	outfd = 1;
    }

    if (skipval) {
	if (lseek(infd, skipval * blocksize, 0) < 0) {
	    while (skipval-- > 0) {
		incc = read(infd, buf, blocksize);
		if (incc < 0) {
		    perror(infile);
		    goto cleanup;
		}
		if (incc == 0) {
		    fprintf(stderr, "End of file while skipping\n");
		    goto cleanup;
		}
	    }
	}
    }
    if (seekval) {
	if (lseek(outfd, seekval * blocksize, 0) < 0) {
	    perror(outfile);
	    goto cleanup;
	}
    }
    if(count != 0x7fffffff)
        inmax = count * blocksize;
    while ((incc = read(infd, buf, blocksize)) > 0) {
	intotal += incc;
	cp = buf;

	if (intflag) {
	    fprintf(stderr, "Interrupted\n");
	    goto cleanup;
	}
	while (incc > 0) {
	    outcc = write(outfd, cp, incc);
	    if (outcc < 0) {
		perror(outfile);
		goto cleanup;
	    }
	    outtotal += outcc;
	    cp += outcc;
	    incc -= outcc;
	}
        if(inmax && intotal >= inmax)
            break;
    }

    if (incc < 0) {
	perror(infile);
	goto cleanup;
    }

    ret = 0;

  cleanup:
    close(infd);

    if (close(outfd) < 0) {
	perror(outfile);
	ret = 1;
    }

    if (buf != localbuf)
	free(buf);

    printf("%d+%d records in\n", intotal / blocksize,
	   (intotal % blocksize) != 0);

    printf("%d+%d records out\n", outtotal / blocksize,
	   (outtotal % blocksize) != 0);

    return ret;
}


/*
 * Read a number with a possible multiplier.
 * Returns -1 if the number format is illegal.
 */
static long getnum(char *cp)
{
    long value;

    if (!isdigit(*cp))
	return -1;

    value = 0;
    while (isdigit(*cp))
	value = value * 10 + *cp++ - '0';

    switch (*cp++) {
    case 'k':
	value *= 1024;
	break;

    case 'b':
	value *= 512;
	break;

    case 'w':
	value *= 2;
	break;

    case '\0':
	return value;

    default:
	return -1;
    }

    if (*cp)
	return -1;

    return value;
}


/* END CODE */
