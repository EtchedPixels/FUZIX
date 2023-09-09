/*
 * Assembler.
 * Command line processing
 * and main driver.
 *
 * FIXME: normal Unix as option parsing.
 */
#include "as.h"
#include <unistd.h>

FILE	*ifp;
FILE	*ofp;
FILE	*lfp;
char	eb[NERR];
char	ib[NINPUT];
char	lb[NINPUT];
char	*cp;
char	*ep;
char	*ip;
char	*fname;
char 	*listname;
VALUE	dot[OSEG];
int	segment = 1;
SYM	*phash[NHASH];
SYM	*uhash[NHASH];
int	pass;
int	line;
jmp_buf	env;
int	debug_write = 1 ;
int	noobj;
int	cpu_flags = ARCH_CPUFLAGS;

static void usage(void)
{
	fprintf(stderr, "as [-o object.o] source.s.\n");
	exit(1);
}

static void oom(void)
{
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}

static char *xstrdup(const char *p)
{
	char *n = strdup(p);
	if (!n)
		oom();
	return n;
}

static int listbytes;

static void list_beginline(void)
{
	if (pass !=3 || !lfp)
		return;
	strcpy(lb, ib);	/* Save the input buffer */
	fprintf(lfp, "%1X %04X : ",
		segment, dot[segment]);
	listbytes = 0;
}

void list_addbyte(uint8_t byte)
{
	if (pass == 3 && lfp) {
		/* Deal with wrapping nicely (eg for .ascii) */
		if (listbytes == 8) {
			fputs(lb, lfp);
			strcpy(lb, "...\n");
			listbytes = 0;
			fprintf(lfp, "%1X %04X : ",
				segment, dot[segment]);
		}
		listbytes++;
		fprintf(lfp, "%02X ", byte);
	}
}

void list_endline(void)
{
	if (pass == 3 && lfp) {
		while(listbytes++ < 8)
			fputs("   ", lfp);
		fputs(lb, lfp);
	}
}

int main(int argc, char *argv[])
{
	char *ifn;
	char *ofn = NULL;
	char *p, *e;

	int opt;

	/* Lots of options need adding yet */
	while ((opt = getopt(argc, argv, "o:l:")) != -1) {
		switch (opt) {
		case 'o':
			ofn = optarg;
			break;
		case 'l':
			listname = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	if (optind != argc - 1)
		usage();
	ifn = argv[optind];

	if ((ifp=fopen(ifn, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open\n", ifn);
		exit(BAD);
	}

	if (ofn == NULL) {
		ofn = xstrdup(ifn);
		p = strrchr(ofn, '.');
		if (p == NULL || p[1] == 0) {
			fprintf(stderr, "%s: expected extensions.\n", ifn);
			exit(BAD);
		}
		p[1] = 'o';
		p[2] = '\0';
	}

	if ((ofp=fopen(ofn, "w")) == NULL) {
		fprintf(stderr, "%s: cannot create.\n", ofn);
		exit(BAD);
	}
	if (listname) {
		lfp = fopen(listname, "w");
		if (lfp == NULL) {
			fprintf(stderr, "%s: cannot create.\n", listname);
			exit(BAD);
		}
	}

	syminit();
	fname = xstrdup(ifn);
	for (pass=0; pass<4; ++pass) {
		if (outpass() == 0)
			continue;
		line = 1;
		memset(dot, 0, sizeof(dot));
		fseek(ifp, 0L, 0);
		while (fgets(ib, NINPUT, ifp) != NULL) {
			/* Pre-processor output */
			if (*ib == '#' && ib[1] == ' ') {
				free(fname);
				line = strtoul(ib +2, &p, 10);
				p++;
				e = strrchr(p , '"');
				if (e)
					*e = 0;
				fname = xstrdup(p);
			/* Normal assembly */
			} else {
				list_beginline();
				ep = &eb[0];
				ip = &ib[0];
				if (setjmp(env) == 0)
					asmline();
				list_endline();
				++line;
			}
		}
		/* Don't continue once we know we failed */
		if (noobj)
			break;
	}
	if (!noobj) {
		pass = 3;
		outeof();
	} else {
		if (unlink(ofn))
			perror(ofn);
	}
	/* Return an error code if no object was created */
	exit(noobj);
}

