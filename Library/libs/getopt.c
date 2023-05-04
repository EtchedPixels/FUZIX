/*
 * From: gwyn@brl-tgr.ARPA (Doug Gwyn <gwyn>) Newsgroups: net.sources
 * Subject: getopt library routine Date: 30 Mar 85 04:45:33 GMT
 */
/*
 * getopt -- public domain version of standard System V routine
 * 
 * Strictly enforces the System V Command Syntax Standard; provided by D A
 * Gwyn of BRL for generic ANSI C implementations
 * 
 * #define STRICT to prevent acceptance of clustered options with arguments
 * and ommision of whitespace between option and arg.
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>

int   opterr = 1;		/* error => print message */
int   optind = 1;		/* next argv[] index */
int   optopt;
char *optarg = NULL;	/* option parameter if any */

static int Err(const char *name, const char *mess, int c)
/* returns '?' */
{
   if (opterr)
   {
      fprintf(stderr,
		     "%s: %s -- %c\n",
		     name, mess, c
	  );
   }
   optopt = c;
   return '?';			/* erroneous-option marker */
}

/* Moved out of function to stop SDCC generating loads of setup crap */
static int sp = 1;		/* position within argument */

int getopt(int argc, char *const argv[], const char *optstring)
				/* returns letter, '?', EOF */
{
#ifdef STRICT
   register int osp;		/* saved `sp' for param test */
#else
   register int oind;		/* saved `optind' for param test */
#endif
   register int c;		/* option letter */
   register char *cp;		/* -> option in `optstring' */

   optarg = NULL;

   if (sp == 1) {		/* fresh argument */
      if (optind >= argc	/* no more arguments */
	  || argv[optind][0] != '-'	/* no more options */
	  || argv[optind][1] == '\0'	/* not option; stdin */
	  )
	 return EOF;
      else if (strcmp(argv[optind], "--") == 0)
      {
	 ++optind;		/* skip over "--" */
	 return EOF;		/* "--" marks end of options */
      }
   }

   c = argv[optind][sp];	/* option letter */
#ifdef STRICT
   osp = sp++;			/* get ready for next letter */
#else
   sp++;
#endif

#ifndef STRICT
   oind = optind;		/* save optind for param test */
#endif
   if (argv[optind][sp] == '\0')/* end of argument */
   {
      ++optind;			/* get ready for next try */
      sp = 1;			/* beginning of next argument */
   }

   if (c == ':' || c == '?'	/* optstring syntax conflict */
       || (cp = strchr(optstring, c)) == NULL	/* not found */
       )
      return Err(argv[0], "illegal option", c);

   if (cp[1] == ':')		/* option takes parameter */
   {
#ifdef STRICT
      if (osp != 1)
	 return Err(argv[0],
		    "option must not be clustered",
		    c
	     );

      if (sp != 1)		/* reset by end of argument */
	 return Err(argv[0],
		    "option must be followed by white space",
		    c
	     );

#else
      if (oind == optind)	/* argument w/o whitespace */
      {
	 optarg = &argv[optind][sp];
	 sp = 1;		/* beginning of next argument */
      }

      else
#endif
      if (optind >= argc)
	 return Err(argv[0],
		    "option requires an argument",
		    c
	     );

      else			/* argument w/ whitespace */
	 optarg = argv[optind];

      ++optind;			/* skip over parameter */
   }

   return c;
}
