/*
 **************************************************************************
 *
 * Utility program to optimize the output of the BCC compiler
 *
 * Module:  copt.c
 * Purpose: Optimize BCC assembler output
 * Entries: main
 *
 * This program is based on an idea from Christopher W. Fraser.
 *
 **************************************************************************
 *
 * Copyright (C) 1995,1996,1997 Gero Kuhlmann <gero@gkminix.han.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define KEEPCOMMENTS

#define MAXLINE		1024
#define HASHSIZE	107
#define NOCHAR		'\177'
#define VARNUM		10


/* Struct containing each string of an input file */
struct line_s {
	char          *text;
	struct line_s *prev;
	struct line_s *next;
	int 	       comment_flg;
};


/* Struct containing one rule */
struct rule_s {
	struct line_s *old;
	struct line_s *new;
	struct rule_s *next;
};


/* Hash table to store strings in a space saving way */
struct hash_s {
	char          *text;
	struct hash_s *next;
};



/*
 * Global variables
 */
static struct rule_s *first = NULL;	/* first rule */
static struct rule_s *last = NULL;	/* last rule */
static struct line_s *infile = NULL;	/* list of strings in input file */
static struct hash_s *htab[HASHSIZE];	/* string hash table */
static int hash_init = 0;		/* flag if hash table initialized */
static char *vars[VARNUM];		/* variable table */
static char *progname;			/* program name */



/*
 * Allocate memory and print error if none available
 */
static void *mymalloc(int size)
{
  void *p;

  if ((p = malloc(size)) == NULL) {
	fprintf(stderr, "%s: no memory\n", progname);
	exit(1);
  }
  return(p);
}



/*
 * Insert a string into the hash table. If the string is already in there
 * just return the pointer to that string.
 */
static char *install(char *str, int slen)
{
  struct hash_s *hp;
  char *chkstr;
  char *cp;
  unsigned int hashval;

  /* Clear the hashing table if not already done */
  if (!hash_init) {
	for (hashval = 0; hashval < HASHSIZE; hashval++)
		htab[hashval] = NULL;
	hash_init++;
  }

  /* Get check string */
  if (slen < 0)
	slen = strlen(str);
  chkstr = mymalloc(slen + 1);
  strncpy(chkstr, str, slen);
  chkstr[slen] = '\0';

  /* Determine hashing value of string */
  hashval = 0;
  for (cp = chkstr; *cp; cp++)
	hashval = hashval*75 + *cp;
  hashval %= HASHSIZE;

  /* Check if the string is already in the hashing table */
  for (hp = htab[hashval]; hp != NULL; hp = hp->next)
	if (!strcmp(chkstr, hp->text)) {
		free(chkstr);
		return(hp->text);
	}

  /* String is not in hash table, so create a new entry */
  hp = (struct hash_s *)mymalloc(sizeof(struct hash_s));
  hp->text = chkstr;
  hp->next = htab[hashval];
  htab[hashval] = hp;
  return(hp->text);
}



/*
 * Read one line from input file and skip all blanks at the beginning
 */
static char *readline(FILE *fp)
{
  static char buf[MAXLINE];
  char *cp;

  /* Read line from input file */
  if (fgets(buf, MAXLINE-1, fp) == NULL)
	return(NULL);
  buf[MAXLINE-1] = '\0';

  /* Delete trailing newline */
  if ((cp = strchr(buf, '\n')) != NULL)
	*cp = '\0';

  /* Delete leading white spaces */
  for (cp = buf; *cp && isspace(*cp); cp++) ;
  if (cp != buf && *cp)
	memmove(buf, cp, strlen(cp) + 1);

  return(buf);
}



/*
 * Read a list of input lines. Terminate reading when the 'quit' character
 * has been found in the first column of the input line. All lines with the
 * 'comment' character in the first position will be skipped.
 */
static struct line_s *readlist(FILE *fp, int quit, int comment)
{
  struct line_s *lp;
  struct line_s *first_line = NULL;
  struct line_s *last_line = NULL;
  char *cp;

  while ((cp = readline(fp)) != NULL) {
	if (quit != NOCHAR && quit == *cp)
		break;
	if (comment != NOCHAR && comment == *cp)
		if (quit != NOCHAR)
			continue;
	if (*cp == '\0')
		continue;
	lp = mymalloc(sizeof(struct line_s));
	lp->text = install(cp, -1);
	lp->prev = last_line;
	lp->next = NULL;
	lp->comment_flg = (comment != NOCHAR && *cp == comment);
	if (first_line == NULL)
		first_line = lp;
	if (last_line != NULL)
		last_line->next = lp;
	last_line = lp;
  }
  return(first_line);
}



/*
 * Read pattern file
 */
static void readpattern(char *rulesdir, char *filename)
{
  static char path[MAXLINE];
  struct rule_s *rp;
  FILE *fp;

  /* Open pattern file */
  if (rulesdir != NULL)
	sprintf(path, "%s/%s", rulesdir, filename);
  else
	sprintf(path, "%s", filename);
  if ((fp = fopen(path, "r")) == NULL) {
	fprintf(stderr, "%s: can't open pattern file %s\n", progname, path);
	exit(1);
  }

  /* Read every line of the pattern file */
  while (!feof(fp)) {
	rp = (struct rule_s *)mymalloc(sizeof(struct rule_s));
	rp->old = readlist(fp, '=', '#');
	rp->new = readlist(fp, '\0', '#');
	if (rp->old == NULL || rp->new == NULL) {
		free(rp);
		break;
	}

/* This put the rules into the table in reverse order; this is confusing *
	rp->next = first;
	first = rp;
	if (last == NULL)
		last = rp;
*/
	rp->next = NULL;
	if (last) {
		last->next = rp;
		last = rp;
	} else {
		first = last = rp;
	}

  }

  /* Close pattern file */
  (void)fclose(fp);
}



/*
 * Clear pattern list to allow for another run
 */
static void clearpattern(void)
{
  struct rule_s *rp1, *rp2;
  struct line_s *lp1, *lp2;

  rp1 = first;
  while (rp1 != NULL) {
	/* Clear old rule text list */
	lp1 = rp1->old;
	while (lp1 != NULL) {
		lp2 = lp1;
		lp1 = lp1->next;
		free(lp2);
	}
	/* Clear new rule text list */
	lp1 = rp1->new;
	while (lp1 != NULL) {
		lp2 = lp1;
		lp1 = lp1->next;
		free(lp2);
	}
	/* Clear rule itself */
	rp2 = rp1;
	rp1 = rp1->next;
	free(rp2);
  }

  first = NULL;
  last = NULL;
}



/*
 * Read input file
 */
static void readinfile(char *filename, int comment)
{
  FILE *fp;

  fp = stdin;
  if (filename != NULL && (fp = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "%s: can't open input file %s\n", progname, filename);
	exit(1);
  }
  infile = readlist(fp, NOCHAR, comment);
  if (fp != stdin)
	  (void)fclose(fp);
}

#define NO_OP  0
#define ADD_OP 1
#define SUB_OP 2
#define MUL_OP 3
#define DIV_OP 4
#define SHL_OP 5
#define SHR_OP 6

  long retval = 0;
  long num = 0;
  int sign = 1;
  int base = 10;
  int op = NO_OP;

/* Apply operation to current numeric value */
static void doretval(void)
{
      switch (op) {
	      case NO_OP:	retval = num * sign;
			      break;
	      case ADD_OP:	retval += num * sign;
			      break;
	      case SUB_OP:	retval -= num * sign;
			      break;
	      case MUL_OP:	retval *= num * sign;
			      break;
	      case DIV_OP:	retval /= num * sign;
			      break;
	      case SHL_OP:	retval <<= num;
			      break;
	      case SHR_OP:	retval >>= num;
			      break;
      }
      op = NO_OP;
      num = 0;
      sign = 1;
      base = 10;
}


/*
 * Eval an expression into an integer number
 */
static long eval(char *str, int len)
{
  char *cp, c;
  int state = 0;
  int i, varnum;

  retval = 0;
  num = 0;
  sign = 1;
  base = 10;
  op = NO_OP;

  /* Scan through whole string and decode it */
  for (cp = str, i = 0; *cp && i < len; cp++, i++) {
	c = toupper(*cp);
	if (c == '-' && (state == 0 || state == 5)) {
		state = 1;
		sign = -1;
	} else if (c == '+' && (state == 0 || state == 5)) {
		state = 1;
		sign = 1;
	} else if (c == '%' && isdigit(*(cp + 1)) && (state < 2 || state == 5)) {
		state = 4;
		varnum = *(cp + 1) - '0';
		if (vars[varnum] == NULL || i >= len)
			return(0);
		num = eval(vars[varnum], strlen(vars[varnum]));
		doretval();
		cp++; i++;
	} else if (c == '$' && (state < 2 || state == 5)) {
		state = 2;
		base = 16;
	} else if (base == 10 && (c >= '0' && c <= '9') &&
	           (state <= 3 || state == 5)) {
		state = 3;
		num = num * 10 + (c - '0');
	} else if (base == 16 &&
	           ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) &&
	           (state <= 3 || state == 5)) {
		state = 3;
		num = num * 16 + (c >= 'A' ? c - '0' - 7 : c - '0');
	} else if (c == ' ' && (state == 3 || state == 4 || state == 5)) {
		if (state == 3) {
			doretval();
			state = 4;
		}
	} else if (strchr("+-*/<>", c) != NULL && (state == 3 || state == 4)) {
		if (state == 3)
			doretval();
		state = 5;
		switch (c) {
			case '+':	op = ADD_OP;
					break;
			case '-':	op = SUB_OP;
					break;
			case '*':	op = MUL_OP;
					break;
			case '/':	op = DIV_OP;
					break;
			case '<':	op = SHL_OP;
					break;
			case '>':	op = SHR_OP;
					break;
		}
	} else
		return(0);
  }

  /* Check if the string has been terminated correctly */
  if (state != 3 && state != 4)
	return(0);
  if (state == 3)
	doretval();
  return(retval);
}



/*
 * Compare an infile string with a pattern string. If there is any variable
 * defined, it will be inserted into the variable list from the pattern
 * string.
 */
static int match(char *ins, char *pat)
{
  char *cp, *oldpat;
  long val;
  int varnum;
  int len = 0;

  while (*ins && *pat)
  {
  	if (pat[0] != '%')
	{
		if (*pat++ != *ins++)
			return(0);
		else
			continue;
	}
	if (pat[1] == '%') {
		/* '%%' actually means '%' */
		if (*ins != '%')
			return(0);
		pat += 2;
	} else if ((pat[1] == '*' || isdigit(pat[1]))) {
		/* Copy variable text into vars array */
		pat += 2;
		for (cp = ins; *ins && !match(ins, pat); ins++) ;
		if (pat[-1] == '*')
			continue;
		len = ins - cp;
		varnum = pat[-1] - '0';
		if (vars[varnum] == NULL)
			vars[varnum] = install(cp, len);
		else if (strlen(vars[varnum]) != len ||
		           strncmp(vars[varnum], cp, len))
			return(0);
	} else if (pat[1] == '[') {
		/* Copy only specific variable text into vars array */
		if ((cp = strchr(pat + 2, ']')) == NULL ||
		    (*(cp + 1) != '*' && !isdigit(*(cp + 1)))) {
			if (*ins != '[')
				return(0);
			pat += 2;
			continue;
		}
		oldpat = pat + 1;
		pat = cp + 2;
		/* Seperate allowable patterns and compare them with ins */
		while (*oldpat && *oldpat != ']') {
			oldpat++;
			len = strcspn(oldpat, "|]");
			if (!strncmp(ins, oldpat, len))
				break;
			oldpat += len;
		}
		if (!*oldpat || *oldpat == ']')
			return(0);
		ins += len;
		if (!match(ins, pat))
			return(0);
		/* Install new string into variable table */
		if (*(cp + 1) == '*')
			continue;
		varnum = *(cp + 1) - '0';
		if (vars[varnum] == NULL)
			vars[varnum] = install(oldpat, len);
		else if (strlen(vars[varnum]) != len ||
		           strncmp(vars[varnum], oldpat, len))
			return(0);
	} else if (pat[1] == '!') {
		/* Match only if the pattern string is not found */
		if (pat[2] != '[' || (cp = strchr(pat + 3, ']')) == NULL) {
			if (*ins != '!')
				return(0);
			pat += 3;
			continue;
		}
		oldpat = pat + 2;
		pat = cp + 1;
		/* Seperate allowable patterns and compare them with ins */
		while (*oldpat && *oldpat != ']') {
			oldpat++;
			len = strcspn(oldpat, "|]");
			if (!strncmp(ins, oldpat, len))
				return(0);
			oldpat += len;
		}
	} else if (pat[1] == '(') {
		/* Match ins with expression */
		if ((cp = strchr(pat + 2, ')')) == NULL) {
			if (*ins != '(')
				return(0);
			pat += 2;
			continue;
		}
		oldpat = pat + 2;
		pat = cp + 1;
		len = cp - oldpat;
		val = eval(oldpat, len);
		for (cp = ins; *ins && !match(ins, pat); ins++) ;
		len = ins - cp;
		if (val != eval(cp, len))
			return(0);
	}
	else /* Bad % format cannot match */
		return(0);
  }

  return(*ins == *pat);
}



/*
 * Substitute variables in a string
 */
static char buf[MAXLINE];

static char *subst(char *pat)
{
  char *cp, *cp1, *cp2, *varptr;
  long num;
  int i = 0;
  int j, pos;

  while (*pat)
	if (pat[0] == '%' && isdigit(pat[1])) {
		/* Substitute with value of variable */
		cp = vars[pat[1] - '0'];
		while (cp != NULL && *cp) {
			buf[i++] = *cp++;
			if (i >= MAXLINE - 1) {
				fprintf(stderr, "%s: line too long\n", progname);
				exit(1);
			}
		}
		pat += 2;
	} else if (pat[0] == '%' && pat[1] == '(') {
		/* Substitute with expression */
		cp = pat + 2;
		if ((pat = strchr(cp, ')')) == NULL || pat - cp <= 0)
			num = 0;
		else
			num = eval(cp, pat - cp);
		if (i >= MAXLINE - 20) {
			fprintf(stderr, "%s: line too long\n", progname);
			exit(1);
		}
		i += sprintf(&buf[i], "%s$%lx", num < 0 ? "-" : "", labs(num));
		pat++;
	} else if (pat[0] == '%' && pat[1] == '{') {
		/* Substitute with expression decimal */
		cp = pat + 2;
		if ((pat = strchr(cp, '}')) == NULL || pat - cp <= 0)
			num = 0;
		else
			num = eval(cp, pat - cp);
		if (i >= MAXLINE - 20) {
			fprintf(stderr, "%s: line too long\n", progname);
			exit(1);
		}
		i += sprintf(&buf[i], "%d", num);
		pat++;
	} else if (pat[0] == '%' && pat[1] == '=') {
		/* Substitute with converted variable */
		/* First seperate all parts of the pattern string */
		cp = pat + 2;
		cp1 = cp2 = varptr = NULL;
		if (*cp == '[') {
			cp1 = ++cp;
			while (*cp && *cp != ']')
				cp++;
			if (cp[0] == ']' && cp[1] == '[') {
				cp += 2;
				cp2 = cp;
				while (*cp && *cp != ']')
					cp++;
				if (cp[0] == ']' && isdigit(cp[1]))
					 varptr = vars[cp[1] - '0'];
			}
		}
		if (cp1 == NULL || cp2 == NULL || varptr == NULL) {
			buf[i++] = *pat++;
			if (i >= MAXLINE - 1) {
				fprintf(stderr, "%s: line too long\n", progname);
				exit(1);
			}
			continue;
		}
		pat = cp + 2;
		/* Now scan through the first string to find variable value */
		cp1--;
		pos = 0;
		while (*cp1 != ']') {
			cp1++;
			j = strcspn(cp1, "|]");
			if (strlen(varptr) == j && !strncmp(cp1, varptr, j))
				break;
			pos++;
			cp1 += j;
		}
		if (*cp1 == ']')
			continue;
		/* Scan through the second string to find the conversion */
		cp2--;
		while (*cp2 != ']' && pos > 0) {
			cp2++;
			j = strcspn(cp2, "|]");
			pos--;
			cp2 += j;
		}
		if (*cp2 == ']' || pos != 0)
			continue;
		/* Insert conversion string into destination */
		cp2++;
		while (*cp2 != '|' && *cp2 != ']') {
			buf[i++] = *cp2++;
			if (i >= MAXLINE - 1) {
				fprintf(stderr, "%s: line too long\n", progname);
				exit(1);
			}
		}
	} else {
		buf[i++] = *pat++;
		if (i >= MAXLINE - 1) {
			fprintf(stderr, "%s: line too long\n", progname);
			exit(1);
		}
	}

  buf[i] = '\0';
  return(install(buf, i));
}



/*
 * Optimize one line of the input file
 */
static struct line_s *optline(struct line_s *cur)
{
  struct rule_s *rp;
  struct line_s *ins, *pat;
  struct line_s *lp1, *lp2;
  int i;

  for (rp = first; rp != NULL; rp = rp->next) {
	/* Clear variable array */
	for (i = 0; i < VARNUM; i++)
		vars[i] = NULL;

	/* Scan through pattern texts and match them against the input file */
	ins = cur;
	pat = rp->old;
	while (ins != NULL
	    && pat != NULL
	    && ( ins->comment_flg ||
	         ( /* (pat->text[0]=='%' || ins->text[0]==pat->text[0]) && */
		  match(ins->text, pat->text)))) {

		if (!ins->comment_flg)
			pat = pat->next;
		else if (ins->text[0]==pat->text[0]) /* Matching a comment! */
		{
		   if (match(ins->text, pat->text))
		      pat = pat->next;
		}
		ins = ins->next;
	}

	/* Current pattern matched input line, so replace input with new */
	if (pat == NULL) {
		/* Clear all lines in the source file for this pattern */
		lp1 = cur;
		cur = cur->prev;
		while (lp1 != ins) {
#if 0
			if( lp1->comment_flg )
			{
				lp2 = lp1;
				lp1 = lp1->next;
				lp2->next = cur->next;
				cur->next = lp2;
				lp2->prev = cur;
				cur=cur->next;
			}
			else
#endif
			{
				lp2 = lp1;
				lp1 = lp1->next;
				free(lp2);
			}
		}
		/* Insert new lines into list */
		pat = rp->new;
		lp1 = cur;
		lp2 = NULL;
		while (pat != NULL) {
			lp2 = mymalloc(sizeof(struct line_s));
			lp2->text = subst(pat->text);
			lp2->next = NULL;
			lp2->prev = lp1;
			lp2->comment_flg = 0;
			if (lp1 != NULL)
				lp1->next = lp2;
			else
				infile = lp2;
			lp1 = lp2;
			pat = pat->next;
		}
		if (ins != NULL)
			ins->prev = lp2;
		if (lp2 != NULL)
			lp2->next = ins;
		else if (lp1 != NULL)
			lp1->next = NULL;
		else
			infile = NULL;
		return(cur);
	}
  }
  return(cur->next);
}



/*
 * Actually optimize all strings in the input file
 */
static void optimize(int backup)
{
  struct line_s *cur, *lp;
  int i;
  int in_asm = 0;

  /* Scan through all lines in the input file */
  cur = infile;
  while (cur != NULL) {
	if (cur->comment_flg || in_asm)
	{
		lp=cur->next;
		if (memcmp(cur->text, "!BCC_", 5) == 0)
			in_asm = (memcmp(cur->text+5, "ASM", 3) == 0);
	}
	else
	        if ((lp = optline(cur)) != NULL && lp != cur->next) {
			for (i = 0; i < backup && lp != NULL; i++)
				lp = lp->prev;
			if (lp == NULL)
				lp = infile;
		}
	cur = lp;
  }
}



/*
 * Write out into destination file
 */
static void writeoutf(char *filename, char *headstr)
{
  FILE *fp;
  struct line_s *lp;

  fp = stdout;
  if (filename != NULL && (fp = fopen(filename, "w")) == NULL) {
	fprintf(stderr, "%s: can't open output file %s\n", progname, filename);
	exit(1);
  }
  if (headstr != NULL) {
	fprintf(fp, "%s", headstr);
	fprintf(fp, "\n");
  }
  for (lp = infile; lp != NULL; lp = lp->next)
	fprintf(fp, "%s\n", lp->text);
  if (fp != stdout)
	  (void)fclose(fp);
}



/*
 * Print usage
 */
static void usage(void)
{
  fprintf(stderr, "usage: %s [-c<comment-char>] [-f<src-file>] [-o<out-file>]\n"
		"\t\t[-b<backup-num>] [-h<head-str>] [-d<ruls-dir>] "
		"<rules-file> ...\n", progname);
  exit(1);
}



/*
 * Main program
 */
int
main(int argc, char **argv)
{
  char comment = NOCHAR;
  char *srcfile = NULL;
  char *outfile = NULL;
  char *headstr = NULL;
  char *rulesdir = NULL;
  int backup = 0;
  int i;

  /* Get program name */
  if ((progname = strrchr(argv[0], '/')) == NULL)
	progname = argv[0];
  else
	progname++;

  /* Make life easy for bcc */
  if ( argc > 4 && strcmp(argv[2], "-o") == 0 && argv[1][0] != '-' )
  {
     srcfile = argv[1];
     argv++,argc--;
  }

  /* Get options from command line */
  for (i = 1; i < argc; i++)
	if (!strncmp(argv[i], "-c", 2))
		comment = argv[i][2];
	else if (!strncmp(argv[i], "-b", 2))
		backup = atoi(&(argv[i][3]));
	else if (!strncmp(argv[i], "-f", 2))
		srcfile = &(argv[i][2]);
	else if (!strncmp(argv[i], "-o", 2))
	{
		if(argv[i][2])
			outfile = &(argv[i][2]);
		else if(++i<argc)
			outfile = argv[i];
		else
			usage();
	}
	else if (!strncmp(argv[i], "-h", 2))
		headstr = &(argv[i][2]);
	else if (!strncmp(argv[i], "-d", 2))
		rulesdir = &(argv[i][2]);
	else if (argv[i][0] == '-')
		usage();
	else
		break;

  /* Have to have enough parameters for rule file names */
  if ((argc - i) < 1)
	usage();

  /* Read source file and optimze it with every rules file */
  readinfile(srcfile, comment);
  for ( ; i < argc; i++) {
	readpattern(rulesdir, argv[i]);
	optimize(backup);
	clearpattern();
  }
  writeoutf(outfile, headstr);

  exit(0);
}

