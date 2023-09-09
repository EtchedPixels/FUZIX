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

static void errstr(uint8_t code)
{
	if (code < 10) {
		printf("%c expected.\n", "),]%"[code-1]);
		return;
	}
	printf("%s.\n", etext[code - 10]);
}
	
/*
 * Handle an error.
 * If no listing file, write out
 * the error directly. Otherwise save
 * the error in the error buffer.
 *
 * Will need tweaking once we support .include as we must show the file
 * name then. TODO
 */
void err(char c, uint8_t code)
{
	if (pass > 1) {
		printf("%s: %d: %c: ", fname, line, toupper(c));
		errstr(code);
		noobj = 1;
	}
	if (c == 'q')
		longjmp(env, 1);
}

/*
 * Not really an error any more (we resolve at link time)
 */
void uerr(char *id)
{
}

/*
 * The "a" error is common.
 */
void aerr(uint8_t code)
{
	err('a', code);
}

/*
 * Ditto the "q" error.
 */
void qerr(uint8_t code)
{
	err('q', code);
}

/*
 * The next character
 * in the input must be a comma
 * or it is a fatal error.
 */
void comma(void)
{
	if (getnb() != ',')
		qerr(MISSING_COMMA);
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
		if (isalpha(c) == 0 && c != '_' && c != '.')
			qerr(INVALID_ID);
	}
	p = &id[0];
	do {
		if (p < &id[NCPS]) {
			*p++ = c;
		}
		if ((c = *ip) != '\n')
			++ip;
	} while (c=='\'' || isalnum(c)!=0 || c == '_');
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
		sp->s_segment = UNKNOWN;
		sp->s_number = -1;
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
	/* Lose any \r\n from confused DOS file formats */
	if (c == '\r')
		return '\n';
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

	while ((c = *ip)==' ' || c=='\t' || c == '\r')
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
