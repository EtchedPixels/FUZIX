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
    char	program;	
} regexp;

extern regexp *regcomp __P((char *));
extern int regexec __P((regexp *prog, char *string));
extern void regsub __P((regexp *prog, char *source, char *dest));
extern void regerror __P((char *));

#endif
