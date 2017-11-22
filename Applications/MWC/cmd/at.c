/*
 * At takes a list of commands and arranges for them to be executed at
 * a specified time.  When the commands are executed, the user id, group
 * id, exported shell variables and current directory will all be as they
 * were when at was executed.
 * The format of the at command is
 *	at [-v] [-c command] time [week] [file]
 * or
 *	at [-v] [-c command] time day_of_week [week] [file]
 * or
 *	at [-v] [-c command] time month day_of_month [file]
 * Here the presence of `week' implies that the command should occur
 * on the next week. If the v-flag is specified, then at prints out
 * when the command will be executed.  If the c-flag is specified
 * then the command come from the string `command'.  If a file is
 * specified, then they come from that file.  If none of these is
 * done, then standard input is read.
 * Note that at should not be set uid or set gid.  Anyone can write
 * in the at spooling directory.  Security is maintained because at
 * sets the set uid, set gid and owner execute bits.  Without all of
 * these being set, atrun will not run the script.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <err.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>


#define	NUL	'\0'
#define TRUE	(0 == 0)
#define FALSE	(0 != 0)
#define	STDOUT	1
#define	WEEK	7
#define YEAR	365
#define	LEAPM	1			/* leap month */
#define	YBASE	1900			/* Year bias in struct tm */
#define	FSTDAY	0			/* Week day of 1st day of year 0 */
#define UNKNOWN	-1
#define	MODE	06500			/* set uid, set gid, owner read-exec */
#define	OUTNL	13			/* "yymmddhhmm.aa" */


struct tm	td	= {		/* time to do command */
			0,		UNKNOWN,	UNKNOWN,
			UNKNOWN,	UNKNOWN,	UNKNOWN,
			UNKNOWN,	UNKNOWN,	UNKNOWN
		};
int		wflg	= FALSE,	/* `week' flag */
		vflg	= FALSE;	/* verify flag */
char		*cstr	= NULL,		/* c-flag command string */
		outn[]	= "/usr/spool/at/yymmddhhmm.aa";
FILE		*outfp	= NULL;		/* file to place commands into */
extern char	**environ;

/*
 * Clean is called when an interrupt occurs, or on any error.  Its only
 * function is to unlink the partial file in at's spooling directory.
 */
void clean(int sig)
{
	if (outfp != NULL) {
		fclose(outfp);
		unlink(outn);
	}
	exit(1);
}

/*
 * Die prints a message on stderr and exits with status one.
 */
void die(const char *str, ...)
{
	va_list va;
	va_start(va, str);
	vwarnx(str, va);
	va_end(va);
	clean(0);
}

/*
 * Setsigs simply assures that on an interrupt we will clean up after
 * ourselves so as to avoid leaving junk in at's spooling directory.
 */
void setsigs(void)
{
	sighandler_t fnc;

	fnc = signal(SIGINT, SIG_IGN);
	if (fnc != SIG_IGN)
		signal(SIGINT, clean);
}

/*
 * Makefile makes a file in the correct directory with a name of the
 * form
 *	yymmddhhmm.xx
 * where
 *	yy is the year (mod 100)
 *	mm is the month
 *	dd is the day of the month
 *	hh is the hour
 * and
 *	mm is the minute
 * at which the command is to be execute.  The xx is simply to make
 * the file name unique.  The name is placed in outn and the stream
 * is in outfp.  Also, the mode of the file is execute for owner only.
 * An initial line is written on the file to remove itself.
 */

void makefile(void)
{
	char *cp;

	cp = &outn[(sizeof outn) - OUTNL - 1];
	sprintf(cp, "%02d%02d%02d%02d%02d.aa", td.tm_year % 100,
		1 + td.tm_mon, td.tm_mday, td.tm_hour, td.tm_min);
	cp = &outn[(sizeof outn) - 2];
	while (access(outn, 0) == 0)
		if (++*cp > 'z') {
			*cp = 'a';
			if (++cp[-1] > 'z')
				die("Too many things to do at the same time");
		}
	outfp = fopen(outn, "w");
	if (outfp == NULL)
		die("Can't create %s\n", outn);
	chmod(outn, MODE);
	fprintf(outfp, "rm -f %s\n", outn);
}

/*
 * Copyenv writes onto outfp a pair of lines for each item in the environment.
 * These lines are of the form
 *	var=value
 *	export var
 * This will cause all exported variables to be reset when the file
 * is executed by the shell.
 */

void copyenv(void)
{
	char **vp, *cp, ch, *end;

	for (vp=environ; (cp=*vp) != NULL; ++vp) {
		while ((ch=*cp++) != '=' && ch != NUL)
			putc(ch, outfp);
		end = cp - 1;
		fprintf(outfp, "='");
		if (ch != '=')
			--cp;
		while ((ch=*cp++) != NUL)
			if (ch == '\'')
				fprintf(outfp, "'\\''");
			else
				putc(ch, outfp);
		fprintf(outfp, "'\n");
		fprintf(outfp, "export %.*s\n", end-*vp, *vp);
	}
}

/*
 * Copyumask writes out a line onto the stream outfp of the form
 *	umask number
 * where number is the current umask.  This will cause the shell
 * to execute the commands with the current umask.
 */
void copyumask(void)
{
	int um;

	um = umask(0777);
	umask(um);
	fprintf(outfp, "umask %03o\n", um);
}

/*
 * Copywd writes a line onto the file outfp of the form
 *	cd current working directory
 * This will cause the current directory to be reset when the file
 * is executed by the shell.
 *
 * FIXME: quoting ?
 */
void copywd(void)
{
	int pid;
	int stat;
	static char *const args[] = {
				"pwd",
				NULL
			};

	fprintf(outfp, "cd ");
	fflush(outfp);
	pid = fork();
	if (pid == 0) {
		dup2(fileno(outfp), STDOUT);
		fclose(outfp);
		execv("/bin/pwd", args);
		execv("/usr/bin/pwd", args);
		exit(1);
	}
	if (pid < 0)
		die("Try again");
	wait(&stat);
	if (stat != 0)
		die("Can't find pwd");
}

/*
 * Copycmds simply copies from stdin until EOF.
 */
void copycmds(void)
{
	int ch;

	if (cstr != NULL)
		fprintf(outfp, "%s\n", cstr);
	else
		while ((ch=getchar()) != EOF)
			putc(ch, outfp);
}

/*
 * Usage give the usage message and dies.
 */
void usage(void)
{
	static char	umsg[]	=
"usage: at [-v] [-c command] time [month month_day | week_day] [file]";

	die(umsg);
}

/*
 * Mopts cracks the `minus' options.  It returns the first
 * unused argument.
 */

char **mopts(char *argv[])
{
	register char	*str,
			ch;

	for (str=*argv; str != NULL  &&  *str++ == '-'; str=*++argv)
		for (ch=*str++; ch != NUL; ch=*str++)
			switch (ch) {
			case 'c':
				if ((cstr = *++argv) == NULL)
					usage();
				break;
			case 'v':
				vflg = TRUE;
				break;
			default:
				usage();
			}
	return (argv);
}

/*
 * Gettime takes the string `str' and sets the td minute and
 * hour fields appropriately.  A time is:
 *	either
 *		0-2 digits (hours)
 *	or
 *		3-4 digits (hours and minutes)
 *	possibly followed by a string starting with
 *		a	for A.M.
 *		p	for P.M.
 *		n	for noon
 *		m	for mid-night.
 */
void gettime(char *str)
{
	register int	ch,
			hour;
	int		min;
	char		*chp;

	hour = 0;
	chp = str;
	for (ch=*chp++; isascii(ch) && isdigit(ch); ch = *chp++)
		hour = 10*hour + ch - '0';
	if (chp - str - 1 <= 2)
		hour *= 100;
	min = hour % 100;
	hour /= 100;
	if (hour >= 24 || min >= 60)
		usage();
	switch (ch) {
	case NUL:
		break;
	case 'a':
		if (hour == 12)
			hour = 0;
		break;
	case 'p':
		if (hour < 12)
			hour += 12;
		break;
	case 'n':
		hour = 12;
		break;
	case 'm':
		hour = 0;
		break;
	default:
		usage();
	}
	td.tm_hour = hour;
	td.tm_min = min;
}

/*
 * Mut searches the table `tbl' for entrys which may be trunctated to
 * `str'.  If there is exactly one such entry, it returns its ordinal.
 * Otherwise it returns EOF.  Note that `tbl' should be NULL-terminated.
 */
int mut(const char *str, const char *tbl[])
{
	register const char **tp;
	register int	len;
	int		res;

	tp = tbl;
	len = strlen(str);
	do {
		if (*tp == NULL)
			return (EOF);
	} while (strncmp(*tp++, str, len) != 0);
	res = tp - tbl - 1;
	do {
		if (*tp == NULL)
			return (res);
	} while (strncmp(*tp++, str, len) != 0);
	return (EOF);
}

/*
 * Getdate fills in either the month and month-day or the week-day
 * fields of td from the date pointed to by `argv'.  It returns
 * a pointer to the next argument.
 */
char	**getdate(char *argv[])
{
	register int	idx;
	static const char	*day[]	= {
				"sunday",	"monday",
				"tuesday",	"wednesday",
				"thursday",	"friday",
				"saturday",	NULL
			},
			*month[]	= {
				"january",	"february",
				"march",	"april",	
				"may",		"june",	
				"july",		"august",
				"september",	"october",	
				"november",	"december",	
				NULL
			};

	if (*argv == NULL)
		return (argv);
	idx = mut(*argv, month);
	if (idx != EOF) {
		td.tm_mon = idx;
		if (*++argv == NULL || (td.tm_mday=atoi(*argv++)) == 0)
			usage();
		return (argv);
	}
	idx = mut(*argv, day);
	if (idx != EOF) {
		td.tm_wday = idx;
		++argv;
	}
	if (*argv != NULL && (wflg = strcmp(*argv, "week")==0))
		++argv;
	return (argv);
}

/*
 * Isleap returns TRUE iff the year `year' is a leap year.
 */
int isleap(int year)
{
	return (year%4 == 0  &&  (year%100 != 0 || year%400 == 0));
}

/*
 * The array mlen is an array of month lengths, indexed by the
 * month number (0 - 11).
 */
static int	mlen[]	= {		/* month lengths */
			31,	28,	31,	30,
			31,	30,	31,	31,
			30,	31,	30,	31
		};


/*
 * Mdtoyd returns the year-day for the date with year `year', month
 * `month' and month-day `mday'.
 */
int mdtoyd(int year, int month, int mday)
{
	int res, leap;

	res = mday - 1;
	if (leap = isleap(year))
		++mlen[LEAPM];
	while (--month >= 0)
		res += mlen[month];
	if (leap)
		--mlen[LEAPM];
	return (res);
}


/*
 * Ydtowd returns the week day for the date with year `year' and
 * year-day `yday'.
 */
int ydtowd(int year, int yday)
{
	register int	wday;

	--year;
	wday = FSTDAY  +  year * (YEAR%WEEK) + yday%WEEK;
	wday += (year+4) / 4;
	wday -= (year+100) / 100;
	wday += (year+400) / 400;
	wday %= WEEK;
	return (wday);
}


/*
 * Ydtom sets `pmonth' and `pmday' to the month and month-day of the
 * date with year `year' and year-day `yday'.
 */
int ydtom(int year, int yday, int *pmonth, int *pmday)
{
	int mday, month;
	int leap;

	if (leap = isleap(year))
		++mlen[LEAPM];
	mday = yday;
	for (month=0; mday >= mlen[month]; ++month)
		mday -= mlen[month];
	*pmonth = month;
	*pmday = mday + 1;
	if (leap)
		--mlen[LEAPM];
}

/*
 * Cyday computes the year and year-day on which the command should be
 * performed.  It does this on the basis of wflg (which is true iff
 * we should delay by a week) and td.
 * The result is placed in td.
 */
void cyday(void)
{
	int late, len;
	struct tm *ct;		/* current time */
	time_t now;

	time(&now);
	ct = localtime(&now);
	td.tm_yday = ct->tm_yday;
	td.tm_year = ct->tm_year;
	if (td.tm_mon != UNKNOWN) {
		late = td.tm_mon <= ct->tm_mon;
		late &= td.tm_mon < ct->tm_mon
			|| td.tm_mday < ct->tm_mday;
		if (late)
			++td.tm_year;
		td.tm_yday = mdtoyd(td.tm_year + YBASE, td.tm_mon, td.tm_mday);
		td.tm_wday = ydtowd(td.tm_year + YBASE, td.tm_yday);
	} else {
		late = td.tm_hour <= ct->tm_hour;
		late &= td.tm_hour < ct->tm_hour
			|| td.tm_min <= ct->tm_min;
		if (td.tm_wday != UNKNOWN) {
			td.tm_yday += (td.tm_wday + WEEK - ct->tm_wday) % WEEK;
			late &= td.tm_wday == ct->tm_wday;
			if (late | wflg)
				td.tm_yday += WEEK;
		} else {
			td.tm_wday = ct->tm_wday;
			if (wflg)
				td.tm_yday += WEEK;
			else if (late) {
				++td.tm_yday;
				td.tm_wday = (td.tm_wday+1) % WEEK;
			}
		}
		len = (isleap(td.tm_year + YBASE)) ? YEAR+1 : YEAR;
		if (td.tm_yday >= len) {
			td.tm_yday -= len;
			++td.tm_year;
		}
		ydtom(td.tm_year + YBASE, td.tm_yday, &td.tm_mon, &td.tm_mday);
	}
}

/*
 * Options cracks the command line options.
 */

void options(char *argv[])
{
	argv = mopts(++argv);
	if (*argv == NULL)
		usage();
	gettime(*argv++);
	argv = mopts(argv);
	argv = getdate(argv);
	argv = mopts(argv);
	if (*argv != NULL) {
		if (cstr != NULL)
			usage();
		if (freopen(*argv, "r", stdin) == NULL)
			die("Can't open %s", *argv);
		argv = mopts(++argv);
	}
	if (*argv != NULL)
		usage();
	cyday();
}


int main(int argc, char *argv[])
{
	setsigs();
	options(argv);
	if (vflg)
		printf("%s", asctime(&td));
	makefile();
	copyenv();
	copyumask();
	copywd();
	copycmds();
	return (0);
}

