/*
 * Z-80 assembler.
 * Symbol table routines
 * and error handling.
 */
#include	"as.h"

/*
 * Given a pointer to a
 * sumbol, compute the hash bucket
 * number. The "add all the characters
 * and take the result modulo the table
 * size" algorithm is used.
 */
int symhash(char *id)
{
	int hash;
	int n;

	hash = 0;
	n = NCPS;
	do {
		hash += *id++;
	} while (--n);
	return (hash&HMASK);
}

/*
 * Handle an error.
 * If no listing file, write out
 * the error directly. Otherwise save
 * the error in the error buffer.
 */
void err(char c)
{
	if (pass != 0) {
		if (lflag != 0)
			storerror(c);
		else
			printf("%04d %c\n", line, c);
	}
	if (c == 'q')
		longjmp(env, 1);
}

/*
 * This routine is like
 * "err", but it has the "u"
 * code screwed into it, and it
 * prints the name of the identifier
 * "id" in the message.
 */
void uerr(char *id)
{
	if (pass != 0) {
		if (lflag != 0)
			storerror('u');
		else
			printf("%04d u %.*s\n", line, NCPS, id);
	}
}

/*
 * The "a" error is common.
 */
void aerr(void)
{
	err('a');
}

/*
 * Ditto the "q" error.
 */
void qerr(void)
{
	err('q');
}

/*
 * Put the error code
 * "c" into the error buffer.
 * Check that it is not already
 * there.
 */

void storerror(int c)
{
	char *p;

	p = &eb[0];
	while (p < ep)
		if (*p++ == c)
			return;
	if (p < &eb[NERR]) {
		*p++ = c;
		ep = p;
	}
}

/*
 * Read identifier.
 * The character "c" is the first
 * character of the name.
 */
void getid(char *id, int c)
{
	char *p;

	if (c < 0) {
		c = getnb();
		if (isalpha(c) == 0)
			qerr();
	}
	p = &id[0];
	do {
		if (p < &id[NCPS]) {
			if (isupper(c))
				c = tolower(c);
			*p++ = c;
		}
		if ((c = *ip) != '\n')
			++ip;
	} while (c=='\'' || isalnum(c)!=0);
	if (c != '\n')
		--ip;
	while (p < &id[NCPS])
		*p++ = 0;
}

/*
 * Lookup symbol in
 * hash table "htable".
 * If not there, and "cf" is
 * true, create it.
 */
SYM	*lookup(char *id, SYM *htable[], int cf)
{
	SYM *sp;
	int hash;

	hash = symhash(id);
	sp  = htable[hash];
	while (sp != NULL) {
		if (symeq(id, sp->s_id))
			return (sp);
		sp = sp->s_fp;
	}
	if (cf != 0) {
		if ((sp=(SYM *)malloc(sizeof(SYM))) == NULL) {
			fprintf(stderr, "No memory\n");
			exit(BAD);
		}
		sp->s_fp = htable[hash];
		htable[hash] = sp;
		sp->s_type = TNEW;
		sp->s_value = 0;
		symcopy(sp->s_id, id);
	}
	return (sp);
}

/*
 * Compare two names.
 * Each are blocks of "NCPS"
 * bytes. True return if the names
 * are exactly equal.
 */
int symeq(char *p1, char *p2)
{
	int n;

	n = NCPS;
	do {
		if (*p1++ != *p2++)
			return (0);
	} while (--n);
	return (1);
}

/*
 * Copy the characters
 * that make up the name of a
 * symbol.
 */
void symcopy(char *p1, char *p2)
{
	int n;

	n = NCPS;
	do {
		*p1++ = *p2++;
	} while (--n);
}

/*
 * Get the next character
 * from the input buffer. Do not
 * step past the newline.
 */
int get(void)
{
	int c;

	if ((c = *ip) != '\n')
		++ip;
	return (c);
}

/*
 * Get the next non
 * whitespace character from 
 * the input buffer. Do not step
 * past the newline.
 */
int getnb(void)
{
	int c;

	while ((c = *ip)==' ' || c=='\t')
		++ip;
	if (c != '\n')
		++ip;
	return (c);
}

/*
 * Put a character back.
 */
void unget(int c)
{
	if (c != '\n')
		--ip;
}
