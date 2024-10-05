/*
 *	An implementation of ps. Rewritten somewhat from the UZIX ps tool.
 *
 *	Deviations from POSIX
 *
 *	-G: not supported
 *	-u: uses the real uid
 *
 *	The -o option is not supported
 *
 *	Meaningless fields should be '-' but few people do this for all cases
 *	and the standard is at odds with reality. We use '-' where others do
 *	but use 0 for things like F.
 *
 *	We show times as dummies until the kernel gets fixed up to put the
 *	right data in the right places.
 *
 *	Many XSI extensions are not implemented
 *
 *	XSI output formats are produced but not all values are available
 *	XSI says command data from the kernel not via grovelling in swap
 *	should be in [] but we don't bother as we don't go digging in swap
 *	anyway.
 *
 *	BSD ps is a lot nicer in many ways. We should add some BSD functions
 *	if possible.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <proc.h>
#include <pwd.h>
#include <fcntl.h>
#include <time.h>

#define TTY_MAJOR	2		/* Be nice if we didn't have to
					   just know this */

static uint16_t optflags;
#define F_e		1		/* All */
#define F_N		2		/* Negate */
#define F_r		4		/* Running */
#define F_a		8		/* All associated with terminals */
#define F_p		16		/* pid match */
#define F_t		32		/* TTY match */
#define F_u		64		/* UID match */
#define F_l		128		/* Long */

static uint16_t outflags;
#define OF_F		1		/* Flags (octal/additive) */
#define OF_S		2		/* State */
#define OF_UID		4		/* UID */
#define OF_PID		8		/* PID */
#define OF_PPID		16		/* PPID */
#define OF_C		32		/* CPU utilization */
#define OF_PRI		64		/* Priority bigger is lower */
#define OF_NI		128		/* Nice value */
#define OF_ADDR		256		/* Address */
#define OF_SZ		512		/* Size in core */
#define OF_WCHAN	1024		/* Wait channel */
#define OF_STIME	2048		/* Starting time */
#define OF_TTY		4096		/* Controlling tty */
#define OF_TIME		8192		/* Cumulative execution time */
#define OF_CMD		16384		/* Command line */

static const char *month = "JanFebMarAprMayJunJulAugSepOctNovDec";

static unsigned nodesize;		/* From the kernel */

static char mapstat(char s)
{
	switch (s) {
	case P_ZOMBIE:
		return 'Z';
	case P_FORKING:
		return 'F';
	case P_RUNNING:
		return 'R';
	case P_READY:
		return 'R';
	case P_SLEEP:
		return 'S';
	case P_IOWAIT:
		return 'D';
	case P_STOPPED:
		return 'T';
	}
	return '?';
}

static struct p_tab_buffer *ptab;
static pid_t *ppid_slot;
static uid_t uid;
static pid_t pid;
static uint8_t tty;

static pid_t pid_max = 32767;	/* FIXME */
static uid_t uid_max = 32767;	/* FIXME */

#define MAXMATCH 16
struct matchtab {
	uint8_t num;
	uint16_t match[MAXMATCH];
	const char *type;
};

static struct matchtab pidtab = {
	0,
	{ 0, },
	"pid"
};

static struct matchtab uidtab = {
	0,
	{ 0, },
	"uid"
};

static struct matchtab ttytab = {
	0,
	{ 0, },
	"tty"
};

static int match(struct matchtab *tab, uint16_t match)
{
	uint16_t *p = tab->match;
	uint8_t n = tab->num;
	while(n--) {
		if (*p++ == match)
			return 1;
	}
	return 0;
}

static int addmatch(struct matchtab *tab, uint16_t match)
{
	if (tab->num == MAXMATCH) {
		fprintf(stderr, "ps: too many %s matches.\n", tab->type);
		exit(1);
	}
	tab->match[tab->num++] = match;
	return 0;
}

static uint16_t do_pidmatch(char *arg)
{
	unsigned long n;
	errno = 0;
	n = strtoul(arg, NULL, 0);
	if (errno || n == 0 || n > pid_max) {
		fprintf(stderr, "ps: invalid process id.\n");
		exit(1);
	}
	return n;
}

static uint16_t do_uidmatch(char *arg)
{
	unsigned long n;
	struct passwd *pw;

	errno = 0;
	n = strtoul(arg, NULL, 0);
	if (errno) {
		pw = getpwnam(arg);
		if (pw == NULL) {
			fprintf(stderr, "ps: unknown user '%s'.\n", arg);
			exit(1);
		}
		n = pw->pw_uid;
		if (n > uid_max) {
			fprintf(stderr, "ps: invalid process id.\n");
			exit(1);
		}
	}
	return n;
}

static char tbuf[] = "/dev/ttyXXX";

static uint16_t do_ttymatch(char *arg)
{
	char *t;
	struct stat st;

	t = tbuf;
	if (isdigit(*arg))
		strlcpy(tbuf + 8, arg, 3);
	else if (strncmp(arg, "tty", 3) == 0)
		strlcpy(tbuf + 8, arg + 3, 3);
	else
		t = arg;
	if (stat(t, &st) == -1) {
		perror(tbuf);
		exit(1);
	}
	if (major(st.st_rdev) != TTY_MAJOR) {
		fprintf(stderr, "%s: not a tty.\n");
		exit(1);
	}
	return minor(st.st_rdev);
}

static void scan_match(struct matchtab *m, char *arg, uint16_t (*op)(char *))
{
	char *p = arg;
	char *b;
	char *work;

	while((b = strtok_r(p, " \t\n,", &work)) != NULL) {
		p = NULL;
		addmatch(m, op(b));
	}
}

static void add_pidmatch(char *arg)
{
	scan_match(&pidtab, arg, do_pidmatch);
}


static void add_uidmatch(char *arg)
{
	scan_match(&uidtab, arg, do_uidmatch);
}

static void add_ttymatch(char *arg)
{
	scan_match(&ttytab, arg, do_ttymatch);
}

static const char *hdrname[] = {
	"F ",
	"S ",
	"  UID ",
	"  PID ",
	" PPID ",
	" C ",
	"PRI ",
	" NI ",
	" ADDR ",
	"   SZ ",
	"WCHAN ",
	"STIME ",
	"   TTY ",
	"    TIME ",
	"CMD",
	NULL
};

static void print_header(void)
{
	uint16_t i = 1;
	uint8_t n = 0;
	while(hdrname[n]) {
		if (outflags & i)
			fputs(hdrname[n], stdout);
		i <<= 1;
		n++;
	}
	putchar('\n');
}

static int compute_show_process(struct p_tab *pp)
{
	/* -e: select all */
	if (optflags & F_e)
		return 1;

	/* -a: all associated with a terminal */
	if ((optflags & F_a) && pp->p_tty)
		return 1;

	/* -p: PID match */
	if ((optflags & F_p) && match(&pidtab, pp->p_pid))
		return 1;

	/* -u: UID match */
	if ((optflags & F_u) && match(&uidtab, pp->p_uid))
		return 1;

	/* -t: TTY match */
	if ((optflags & F_t) && match(&ttytab, pp->p_tty))
		return 1;

	if ((optflags & F_r) && (pp->p_status != P_RUNNING && pp->p_status != P_READY))
		return 0;
	/* Really we want euid but the euid isn't visible */
	if (pp->p_uid == uid && pp->p_tty == tty)
		return 1;

	return 0;
}

int show_process(struct p_tab *pp)
{
	if (pp->p_status == 0)
		return 0;

	if (optflags & F_N)
		return !compute_show_process(pp);
	else
		return compute_show_process(pp);
}

static const char *username(uid_t uid)
{
	static char uname[6];
	struct passwd *pwd = getpwuid(uid);
	if (pwd)
		return pwd->pw_name;
	sprintf(uname, "%d", uid);
	return uname;
}

static char tname[7] = "tty";

static const char *ttyshortname(uint8_t tty)
{
	sprintf(tname + 3, "%d", tty);
	return tname;
}

void display_process(struct p_tab *pp, int i)
{
	int j;

	if (outflags & OF_F)
		fputs("0 ", stdout);
	if (outflags & OF_S) {
		putchar(mapstat(pp->p_status));
		putchar(' ');
	}
	if (outflags & OF_UID)
		printf("%5d ", pp->p_uid);
	if (outflags & OF_PID)
		printf("%5d ", pp->p_pid);
	if (outflags & OF_PPID)
		printf("%5d ", ptab[ppid_slot[i]].p_tab.p_pid);
	if (outflags & OF_C)
		fputs(" 0 ", stdout);
	if (outflags & OF_PRI)
		printf("%3d ", pp->p_priority);
	if (outflags & OF_NI)
		printf("%3d ", pp->p_nice);
	if (outflags & OF_ADDR)
		fputs("    - ", stdout);
	if (outflags & OF_SZ) {
		if (pp->p_size)
			printf("%5d ", pp->p_size);
		else
			fputs("    - ", stdout);
	}
	if (outflags & OF_WCHAN) {
		if (pp->p_status > 2)
			printf(" %4x ", (unsigned int)pp->p_wait);
		else
			fputs("    - ", stdout);
	}
	/* We need to sort out the whole kernel and user handling of
	   times in ptab verus udata here */
	if (outflags & OF_STIME) {
		time_t t;
		struct tm *tm, *tnow;
		__ktime_t ticks;
		/* ps is not portable code so don't bother with
		   clock_gettimer() */
		_time(&ticks, 1);	/* Get the tick timer */

		/* Get the current time */
		t = time(NULL);
		tnow = localtime(&t);
		/* Make t the unix time at process start according to the clock
		   at this moment in time */
		t -= ticks.low/10;
		t += pp->p_time / 10;

		tm = localtime(&t);

		if (tm->tm_year != tnow->tm_year)
			printf("%04d ", tm->tm_year);
		else if (tm->tm_yday != tnow->tm_yday)
			printf("%.3s%02d",
				month + 3 * tm->tm_mon, tm->tm_mday);
		else
			printf("%02d:%02d ",
				tm->tm_hour, tm->tm_min);
	}
				
	if (outflags & OF_TTY) {
		if (!pp->p_tty)
			fputs("     ?  ", stdout);
		else
			printf("%7s ", ttyshortname(pp->p_tty));
	}
	if (outflags & OF_TIME) {
		/* cstime/cutime or ctime/utime ? */ 
		uint32_t c = pp->p_cstime + pp->p_cutime;
		uint8_t secs = c % 60;
		uint8_t mins;
		c -= secs;
		c /= 60;
		mins = c % 60;
		c -= mins;
		c /= 60;
		/* What to do if we exceed 99 hours ? */
		printf("%02d:%02d:%02d ",
			c, mins, secs);
	}
	if (outflags & OF_CMD) {
		char name[9];
		strncpy(name, pp->p_name, 8);
		name[8] = '\0';

		for (j = 0; j < 8; ++j) {
			if (name[j] != 0)
				if (name[j] < ' ' || name[j] > 126)
					name[j] = '?';
		}
		fputs(name, stdout);
		if (pp->p_status == P_ZOMBIE)
			fputs(" <defunct>", stdout);
		putchar('\n');
	}
}

static unsigned find_pid_slot(unsigned slot)
{
	int diff;
	/* Work out which slot is referenced from the node size as our own ptab struct
	   may not exactly size match the kernel */
	if (ptab[slot].p_tab.p_pptr == NULL)
		return 0;

	diff = (uint8_t *)ptab[slot].p_tab.p_pptr - (uint8_t *)ptab[0].p_tab.p_pptr;
	if (diff % nodesize) {
		fputs("internal nodesize error\n", stderr);
		exit(1);
	}
	return diff / nodesize;
}

int do_ps(void)
{
	int i, pfd, ptsize;
	struct p_tab_buffer *ppbuf;
	struct p_tab *pp;

	uid = geteuid();
	pid = getpid();

	if ((pfd = open("/dev/proc", O_RDONLY)) < 0) {
		perror("ps");
		return 1;
	}

	if (ioctl(pfd, 2, (char *) &nodesize) != 0) {
		perror("ioctl");
		close(pfd);
		return 1;
	}

	if (nodesize > sizeof(struct p_tab_buffer)) {
		fprintf(stderr, "kernel/user include mismatch (%d v %d).\n",
			nodesize, sizeof(struct p_tab_buffer));
		exit(1);
	}

	if (ioctl(pfd, 1, (char *) &ptsize) != 0) {
		perror("ioctl");
		close(pfd);
		return 1;
	}

	ptab = calloc(ptsize, sizeof(struct p_tab_buffer));
	ppid_slot = calloc(ptsize, sizeof(pid_t));
	
	if (ptab == NULL || ppid_slot == NULL) {
		fprintf(stderr, "ps: out of memory.\n");
		exit(1);
	}

	for (i = 0; i < ptsize; ++i) {
		if (read(pfd, (char *) &ptab[i], nodesize) != nodesize) {
			fprintf(stderr, "ps: error reading from /dev/proc\n");
			close(pfd);
			return 1;
		}
		ppid_slot[i] = find_pid_slot(i);
		/* Learn our tty internal reference as we go */
		if (ptab[i].p_tab.p_status && ptab[i].p_tab.p_pid == pid)
			tty = ptab[i].p_tab.p_tty;
	}
	close(pfd);

	print_header();

	for (ppbuf = ptab, i = 0; i < ptsize; ++i, ++ppbuf) {
		pp = &ppbuf->p_tab;

		if (!show_process(pp))
			continue;

		display_process(pp, i);
	}
	return 0;
}

static void usage(void)
{
	fputs("usage: ps [options]\n", stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	int opt;

	optflags = 0;
	outflags = OF_PID|OF_TTY|OF_TIME|OF_CMD;
	
	while((opt = getopt(argc, argv, "AeaNrp:t:U:u:lf")) != -1) {
		switch(opt) {
			case 'A':
			case 'e':
				/* Show everything */
				optflags |= F_e;
				break;
			case 'a':
				/* All except session leaders and not
				   associated with tty. We only do not assoc */
				optflags |= F_a;
				break;
			case 'N':
				/* Invert match */
				optflags |= F_N;
				break;
			/* BSDism : keep if doesn't clash */
			case 'r':
				/* Only running processes */
				optflags |= F_r;
				break;
			case 'p':
				/* Must match this pid */
				optflags |= F_p;
				add_pidmatch(optarg);
				break;
			case 't':
				optflags |= F_t;
				add_ttymatch(optarg);
				break;
			case 'U':
			case 'u':	/* -u deviates from the standard */
				optflags |= F_u;
				add_uidmatch(optarg);
				break;
			case 'l':
				outflags |= OF_F|OF_S|OF_UID|OF_PID|OF_PPID|
					    OF_C|OF_PRI|OF_NI|OF_ADDR|OF_SZ|
					    OF_WCHAN|OF_TTY|OF_TIME|OF_CMD;
				break;
			case 'f':
				outflags |= OF_UID|OF_PID|OF_PPID|OF_C|
					    OF_STIME|OF_TTY|OF_TIME|OF_CMD;
				break;
			default:
				usage();
		}
	}
	if (optind != argc)
		usage();

	return do_ps();
}
