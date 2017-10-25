/*
 * Cron. Execute commands stored in /usr/lib/crontab at preset times.
 * It sets its uid and gid to those of daemon, if possible.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/wait.h>

#define	TRUE	(0 == 0)
#define FALSE	(0 != 0)

/*
 * Time field types.
 */
#define	MIN	0
#define HOUR	1
#define	MDAY	2
#define	MON	3
#define WDAY	4

/*
 * (Finite) States in valid().
 */
#define START	0
#define INT	1
#define INTDASH	2
#define RANGE	3
#define STR	4
#define STRPLUS	5
#define ERR	6
#define END	7
#define GLOB	('*')

/*
 * Tokens returned by gettoken(). Tokens are integers. EOF is also a token.
 * INT, STR and GLOB above are also tokens.
 */
#define DASH	('-')
#define COMMA	(',')
#define WS	(' ')
#define PERCENT	(-2)
#define NEWLINE (-3)

struct	tm *tm;
time_t	clockv;
int	tmfield[5];
FILE	*f;

int	ugtokflag = FALSE;
int	ugtoken;
int	ufetflag = FALSE;
int	ufetval;

char	*tokbuf;
int	buflen = 512;
char	crontab[] = "/usr/lib/crontab";

void oom(void)
{
	write(2, "Out of memory.\n", 15);
	exit(255);
}


void unfetch(int c)
{
	ufetval = c;
	ufetflag = TRUE;
}

int fetch(void)
{
	register int c;
	register int c2;

	if (ufetflag) {
		ufetflag = FALSE;
		return (ufetval);
	}

	for (;;)
		switch (c = getc(f)) {
		case '%':
			return (PERCENT);
		case '\n':
			return (NEWLINE);
		case '\\':
			if ((c2 = getc(f)) == '%')
				return ('%');
			else if (c2 == '\n')
				continue;
			else {
				ungetc(c2, f);
				return ('\\');
			}
		default:
			return (c);
		}
}


void ungettoken(int t)
{
	ugtokflag = TRUE;
	ugtoken = t;
}

int gettoken(void)
{
	register int c;
	register char *sp = tokbuf;
	register char *mark;
	int posn;

	if (ugtokflag) {
		ugtokflag = FALSE;
		return (ugtoken);
	}

	switch (c = fetch()) {
	case NEWLINE:
	case EOF:
	case PERCENT:
		return (c);
	case ' ':
	case '\t':
		while ((c = fetch()) == ' '  ||  c == '\t')
			;
		unfetch(c);
		return (WS);
	case '-':
		return (DASH);
	case ',':
		return (COMMA);
	case '*':
		return (GLOB);
	}


	/*
	 * Case of INT. Place ascii digit string into tokbuf.
	 */
	if (isdigit(c)) {
		*sp++ = c;
		while (isdigit(c = fetch()))
			*sp++ = c;
		unfetch(c);
		*sp = '\0';
		return (INT);
	}

	/*
	 * The only remaining possibility is the token STR or STRPLUS.
	 */
	*sp++ = c;
	mark = tokbuf + buflen - 2;
	while ((c = fetch()) != EOF  &&  c != NEWLINE  &&  c != PERCENT) {
		if (sp == mark) {
			posn = sp - tokbuf;
			tokbuf = realloc(tokbuf, (buflen += 128));
			if (tokbuf == NULL)
				oom();
			sp = tokbuf + posn;
			mark = sp + buflen;
		}
		*sp++ = c;
	}
	*sp = '\0';
	if (c == PERCENT)
		return (STRPLUS);
	unfetch(c);
	return (STR);
}

int skip_it(void)
{
	register int t;

	while ((t = gettoken()) != EOF  &&  t != NEWLINE)
		;
	return (t);
}

int do_it(void)
{
	register int c;
	register FILE *fp;

	if ((c = gettoken()) != STR  &&  c != STRPLUS) {
		ungettoken(c);
		return (skip_it());
	}
	if ((fp = popen(tokbuf, "w")) == NULL) {
		fprintf(stderr, "Cron:\tCould not popen: %s\n", tokbuf);
		perror("popen");
		return (skip_it());
	}
	if (c == STR) {
		fclose(fp);
		return (gettoken());
	}
	while ((c = fetch()) != NEWLINE  &&  c != EOF)
		if (c == PERCENT)
			putc('\n', fp);
		else
			putc(c, fp);
	putc('\n', fp);
	fclose(fp);
	return(c);
}

/*
 * Valid(fieldnum) parses the next time field from the current line in crontab
 * and checks whether tmfield[fieldnum] satisfies the constraints of that time
 * field. Returns TRUE if so, FALSE if not. Leaves f at the next field.
 * Detects syntax errors in the time fields.
 */
int valid(int fieldnum)
{
	register int t;
	register int ival;
	register int state = START;
	int ival2;
	int tm_val = tmfield[fieldnum];

	if (fieldnum == MIN) {
		while ((t = gettoken()) == WS  ||  t == NEWLINE)
			;
		ungettoken(t);
	}

	for (;;) {
		t = gettoken();
		switch (state) {
		case START:
			switch (t) {
			case INT:
				if (strlen(tokbuf) <= 2) {
					ival = atoi(tokbuf);
					state = INT;
				}
				else
					state = ERR;
				break;
			case GLOB:
				state = GLOB;
				break;
			case WS:
				state = END;
				break;
			default:
				state = ERR;
				break;
			}
			break;
		case INT:
			if (t == COMMA  ||  t == WS) {
				if (ival == tm_val) {
					while (t != WS)
						t = gettoken();
					return (TRUE);
				}
				state = (t == WS) ? END : START;
			}
			else if (t == DASH)
				state = INTDASH;
			else
				state = ERR;
			break;
		case INTDASH:
			if (t == INT  &&  strlen(tokbuf) <= 2) {
				ival2 = atoi(tokbuf);
				state = RANGE;
			}
			else
				state = ERR;
			break;
		case RANGE:
			if (t == COMMA  ||  t == WS) {
				if (ival <= tm_val  &&  tm_val <= ival2) {
					while (t != WS)
						t = gettoken();
					return (TRUE);
				}
				state = (t == WS) ? END : START;
			}
			else
				state = ERR;
			break;
		case GLOB:
			if (t == COMMA  ||  t == WS) {
				while (t != WS)
					t = gettoken();
				return (TRUE);
			}
			else
				state = ERR;
			break;
		}		/* End switch on state */

		if (state == END)
			return (FALSE);
		if (state == ERR) {
			ungettoken(t);
			ungettoken(skip_it());
			return (FALSE);
		}
	}
}

/*
 * Test and Execute: tests a crontab entry against the time fields in `tm' to
 * see if it should be executed, if so it executes. f is left pointing to the
 * next entry (line). Returns EOF when encountered, something else otherwise.
 */
int tex(void)
{
	register int fieldnum;

	for (fieldnum = MIN; fieldnum <= WDAY; ++fieldnum)
		if (!valid(fieldnum))
			return (skip_it());
	return (do_it());
}

void catchalarm(int sig)
{
	signal(SIGALRM, catchalarm);
	time(&clockv);
	tm = localtime(&clockv);
	alarm(61 - tm->tm_sec);
}

void sigsetup(void)
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, catchalarm);
}

void todaemon(void)
{
	struct passwd	*pwp;

	pwp = getpwnam("daemon");
	endpwent();
	if (pwp == NULL || setgid(pwp->pw_gid) || setuid(pwp->pw_uid))
		exit(1);
}

int main(int argc, char *argv[])
{
	int n;
	sigsetup();

	todaemon();
	tokbuf = malloc(buflen);
	if (tokbuf == NULL)
		oom();
	chdir("/bin");
	time(&clockv);
	tm = localtime(&clockv);
	alarm(61 - tm->tm_sec);

	for (;;) {
		/* Put the time values into tmfield[] for easy reference.
		 * localtime() gives tm_mon in the range 0-11, tm_wday, 0-6
		 * (0=sunday). These are adjusted for cron's syntax.
		 */
		tmfield[MIN]	= tm->tm_min;
		tmfield[HOUR]	= tm->tm_hour;
		tmfield[MDAY]	= tm->tm_mday;
		tmfield[MON]	= tm->tm_mon + 1;
		tmfield[WDAY]	= tm->tm_wday;

		if ((f = fopen(crontab, "r")) != NULL) {
			while (tex() != EOF)
				;
			fclose(f);
		}
		while (wait(&n) != -1)
			;
		if (errno == ECHILD)
			pause();
	}
}

