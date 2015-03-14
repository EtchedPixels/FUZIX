/*
 * Copyright (c) 1989, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kim Letkeman.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1989, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
static char sccsid[] = "@(#)cal.c	8.4 (Berkeley) 4/2/94";
#endif	/* not lint */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int time_zone = 0;
long timezone = 0;

int errx(int exv, char *str)
{
    fprintf(stderr, "cal: %s\n", str);
    exit(exv);
}

#define	THURSDAY		4	/* for reformation */
#define	SATURDAY 		6	/* 1 Jan 1 was a Saturday */

#define	FIRST_MISSING_DAY 	639787	/* 3 Sep 1752 */
#define	NUMBER_MISSING_DAYS 	11	/* 11 day correction */

#define	MAXDAYS			42	/* max slots in a month array */
#define	SPACE			-1	/* used in day array */

static int days_in_month[2][13] =
{
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

int sep1752[MAXDAYS] =
{
    SPACE, SPACE, 1,     2,     14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
}, j_sep1752[MAXDAYS] =
{
    SPACE, SPACE, 245,   246,   258,   259,   260,
    261,   262,   263,   264,   265,   266,   267,
    268,   269,   270,   271,   272,   273,   274,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
}, empty[MAXDAYS] =
{
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
    SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE,
};

char day_headings[] = "Su Mo Tu We Th Fr Sa ";
char j_day_headings[] = "Sun Mon Tue Wed Thu Fri Sat ";
char *full_month[12] = {
    "January",   "February", "March",    "April",
    "May",       "June",     "July",     "August",
    "September", "October",  "November", "December"
};

/* leap year -- account for gregorian reformation in 1752 */
#define	leap_year(yr) \
	((yr) <= 1752 ? !((yr) % 4) : \
	(!((yr) % 4) && ((yr) % 100)) || !((yr) % 400))

/* number of centuries since 1700, not inclusive */
#define	centuries_since_1700(yr) \
	((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* number of centuries since 1700 whose modulo of 400 is 0 */
#define	quad_centuries_since_1700(yr) \
	((yr) > 1600 ? ((yr) - 1600) / 400 : 0)

/* number of leap years between year 1 and this year, not inclusive */
#define	leap_years_since_year_1(yr) \
	((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

int julian;

void ascii_day(char *, int);
void center(char *, int, int);
void day_array(int, int, int *);
int  day_in_week(int, int, int);
int  day_in_year(int, int, int);
void j_yearly(int);
void monthly(int, int);
void trim_trailing_spaces(char *);
void usage(void);
void yearly(int);

int days[12][MAXDAYS];
char lineout[80];

int main(int argc, char *argv[])
{
    struct tm *local_time;
    time_t now;
    int  ch, month, year, yflag;

    julian = yflag = 0;

#if 1
    while ((ch = getopt(argc, argv, "jy")) != EOF)
	switch (ch) {
	case 'j':
	    julian = 1;
	    break;
	case 'y':
	    yflag = 1;
	    break;
	case '?':
	default:
	    usage();
	}
    argc -= optind;
    argv += optind;
#else
    argc--;
    argv++;
#endif

    month = 0;
    switch (argc) {
    case 2:
	if ((month = atoi(*argv++)) < 1 || month > 12)
	    errx(1, "illegal month value: use 1-12");
	/* FALLTHROUGH */
    case 1:
	if ((year = atoi(*argv)) < 1 || year > 9999)
	    errx(1, "illegal year value: use 1-9999");
	break;

    case 0:
	now = time(NULL);
	local_time = localtime(&now);
	year = local_time->tm_year + 1900;
	if (!yflag)
	    month = local_time->tm_mon + 1;
	break;

    default:
	usage();
    }

    if (month)
	monthly(month, year);
    else if (julian)
	j_yearly(year);
    else
	yearly(year);
    exit(0);
}

#define	DAY_LEN		3	/* 3 spaces per day */
#define	J_DAY_LEN	4	/* 4 spaces per day */
#define	WEEK_LEN	21	/* 7 days * 3 characters */
#define	J_WEEK_LEN	28	/* 7 days * 4 characters */
#define	HEAD_SEP	2	/* spaces between day headings */
#define	J_HEAD_SEP	2

void monthly(int month, int year)
{
    int col, row, len, days[MAXDAYS];
    char *p, lineout[30];

    day_array(month, year, days);
    len = sprintf(lineout, "%s %d", full_month[month - 1], year);
    printf("%*s%s\n%s\n",
	   ((julian ? J_WEEK_LEN : WEEK_LEN) - len) / 2, "",
	   lineout, julian ? j_day_headings : day_headings);
    for (row = 0; row < 6; row++) {
	for (col = 0, p = lineout; col < 7; col++,
	     p += julian ? J_DAY_LEN : DAY_LEN)
	    ascii_day(p, days[row * 7 + col]);
	*p = '\0';
	trim_trailing_spaces(lineout);
	printf("%s\n", lineout);
    }
}

void j_yearly(int year)
{
    char *p;
    int col, *dp, i, month, row, which_cal;

    sprintf(lineout, "%d", year);
    center(lineout, J_WEEK_LEN * 2 + J_HEAD_SEP, 0);
    printf("\n\n");
    for (i = 0; i < 12; i++)
	day_array(i + 1, year, days[i]);
#if 0
    memset(lineout, ' ', sizeof(lineout) - 1);
#else /* Hi-Tech C */
    memset(lineout, sizeof(lineout) - 1, ' ');
#endif
    lineout[sizeof(lineout) - 1] = '\0';
    for (month = 0; month < 12; month += 2) {
	center(full_month[month], J_WEEK_LEN, J_HEAD_SEP);
	center(full_month[month + 1], J_WEEK_LEN, 0);
	printf("\n%s%*s%s\n", j_day_headings, J_HEAD_SEP, "", j_day_headings);
	for (row = 0; row < 6; row++) {
	    for (which_cal = 0; which_cal < 2; which_cal++) {
		p = lineout + which_cal * (J_WEEK_LEN + 2);
		dp = &days[month + which_cal][row * 7];
		for (col = 0; col < 7; col++, p += J_DAY_LEN)
		    ascii_day(p, *dp++);
	    }
	    *p = '\0';
	    trim_trailing_spaces(lineout);
	    printf("%s\n", lineout);
	}
    }
    printf("\n");
}

void yearly(int year)
{
    char *p;
    int col, *dp, i, month, row, which_cal;

    sprintf(lineout, "%d", year);
    center(lineout, WEEK_LEN * 3 + HEAD_SEP * 2, 0);
    printf("\n\n");
    for (i = 0; i < 12; i++)
	day_array(i + 1, year, days[i]);
#if 0
    memset(lineout, ' ', sizeof(lineout) - 1);
#else /* Hi-Tech C */
    memset(lineout, sizeof(lineout) - 1, ' ');
#endif
    lineout[sizeof(lineout) - 1] = '\0';
    for (month = 0; month < 12; month += 3) {
	center(full_month[month], WEEK_LEN, HEAD_SEP);
	center(full_month[month + 1], WEEK_LEN, HEAD_SEP);
	center(full_month[month + 2], WEEK_LEN, 0);
	printf("\n%s%*s%s%*s%s\n", day_headings, HEAD_SEP,
		      "", day_headings, HEAD_SEP, "", day_headings);
	for (row = 0; row < 6; row++) {
	    for (which_cal = 0; which_cal < 3; which_cal++) {
		p = lineout + which_cal * (WEEK_LEN + 2);
		dp = &days[month + which_cal][row * 7];
		for (col = 0; col < 7; col++, p += DAY_LEN)
		    ascii_day(p, *dp++);
	    }
	    *p = '\0';
	    trim_trailing_spaces(lineout);
	    printf("%s\n", lineout);
	}
    }
    printf("\n");
}

/*
 * day_array --
 *      Fill in an array of 42 integers with a calendar.  Assume for a moment
 *      that you took the (maximum) 6 rows in a calendar and stretched them
 *      out end to end.  You would have 42 numbers or spaces.  This routine
 *      builds that array for any month from Jan. 1 through Dec. 9999.
 */
void day_array(int month, int year, int *days)
{
    int day, dw, dm;

    if (month == 9 && year == 1752) {
	memmove(days,
		julian ? j_sep1752 : sep1752, MAXDAYS * sizeof(int));
	return;
    }
    memmove(days, empty, MAXDAYS * sizeof(int));
    dm = days_in_month[leap_year(year)][month];
    dw = day_in_week(1, month, year);
    day = julian ? day_in_year(1, month, year) : 1;
    while (dm--)
	days[dw++] = day++;
}

/*
 * day_in_year --
 *      return the 1 based day number within the year
 */
int day_in_year(int day, int month, int year)
{
    int i, leap;

    leap = leap_year(year);
    for (i = 1; i < month; i++)
	day += days_in_month[leap][i];
    return (day);
}

/*
 * day_in_week
 *      return the 0 based day number for any date from 1 Jan. 1 to
 *      31 Dec. 9999.  Assumes the Gregorian reformation eliminates
 *      3 Sep. 1752 through 13 Sep. 1752.  Returns Thursday for all
 *      missing days.
 */
int day_in_week(int day, int month, int year)
{
    long temp;

    temp = (long) (year - 1) * 365 + leap_years_since_year_1(year - 1)
	+ day_in_year(day, month, year);
    if (temp < FIRST_MISSING_DAY)
	return ((temp - 1 + SATURDAY) % 7);
    if (temp >= (FIRST_MISSING_DAY + NUMBER_MISSING_DAYS))
	return (((temp - 1 + SATURDAY) - NUMBER_MISSING_DAYS) % 7);
    return (THURSDAY);
}

void ascii_day(char *p, int day)
{
    int display, val;
    static char *aday[] =
    {
	"",
	" 1", " 2", " 3", " 4", " 5", " 6", " 7",
	" 8", " 9", "10", "11", "12", "13", "14",
	"15", "16", "17", "18", "19", "20", "21",
	"22", "23", "24", "25", "26", "27", "28",
	"29", "30", "31",
    };

    if (day == SPACE) {
	memset(p, ' ', julian ? J_DAY_LEN : DAY_LEN);
	return;
    }
    if (julian) {
	if ((val = day / 100)) {
	    day %= 100;
	    *p++ = val + '0';
	    display = 1;
	} else {
	    *p++ = ' ';
	    display = 0;
	}
	val = day / 10;
	if (val || display)
	    *p++ = val + '0';
	else
	    *p++ = ' ';
	*p++ = day % 10 + '0';
    } else {
	*p++ = aday[day][0];
	*p++ = aday[day][1];
    }
    *p = ' ';
}

void trim_trailing_spaces(char *s)
{
    char *p;

    for (p = s; *p; ++p)
	continue;
    while (p > s && isspace(*--p))
	continue;
    if (p > s)
	++p;
    *p = '\0';
}

void center(char *str, int len, int separate)
{

    len -= strlen(str);
    printf("%*s%s%*s", len / 2, "", str, len / 2 + len % 2, "");
    if (separate)
	printf("%*s", separate, "");
}

void usage(void)
{
    fprintf(stderr, "usage: cal [-jy] [[month] year]\n");
    exit(1);
}

#if 0
/*------------------------------------------------------------------*/

/*
 * getopt - parse command-line options
 */

#define ERR(s, c)       if(opterr){\
	fputs(argv[0], stderr);\
	fputs(s, stderr);\
	fputc(c, stderr);\
	fputc('\n', stderr);}

int  opterr = 1;
int  optind = 1;
int  optopt;
char *optarg;

int getopt(argc, argv, opts)
int argc;
char **argv;
char *opts;
{
    static int sp = 1;
    register c;
    register char *cp;

    if (sp == 1) {
	if (optind >= argc ||
	    argv[optind][0] != '-' || argv[optind][1] == '\0')
	    return EOF;
	else if (!strcmp(argv[optind], "--")) {
	    optind++;
	    return EOF;
	}
    }

    optopt = c = argv[optind][sp];
    if (c == ':' || (cp=strchr(opts, c)) == NULL) {
	ERR (": illegal option -- ", c);
	if (argv[optind][++sp] == '\0') {
	    optind++;
	    sp = 1;
	}
	return '?';
    }
    if (*++cp == ':') {
	if (argv[optind][sp+1] != '\0') {
	    optarg = &argv[optind++][sp+1];
	} else if (++optind >= argc) {
	    ERR (": option requires an argument -- ", c);
	    sp = 1;
	    return '?';
	} else {
	    optarg = argv[optind++];
	}
	sp = 1;
    } else {
	if (argv[optind][++sp] == '\0') {
	    sp = 1;
	    optind++;
	}
	optarg = NULL;
    }

    return c;
}
#endif
