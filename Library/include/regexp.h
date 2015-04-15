/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef __REGEXP_H
#define __REGEXP_H

#define NSUBEXP  10

typedef struct regexp {
	char	*startp[NSUBEXP];
	char	*endp[NSUBEXP];
	char	regstart;	/* Internal use only. */
	char	reganch;	/* Internal use only. */
	char	*regmust;	/* Internal use only. */
	int	regmlen;	/* Internal use only. */
	char	program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

extern regexp *regcomp(char *);
extern int regexec(regexp *__prog, char *__string);
extern void regsub(regexp *__prog, char *__source, char *__dest);
extern void regerror(const char *);

#endif
