#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "cc.h"

#define CPP_DEBUG 0		/* LOTS of junk to stderr. */

/*
 * This file comprises the 'guts' of a C preprocessor.
 *
 * Functions exported from this file:
 *   gettok() Returns the next token from the source
 *            curword contains the text of the token
 *
 * Variables
 *   curword  Contains the text of the last token parsed.
 *   curfile  Currently open primary file
 *   c_fname  Name of file being parsed
 *   c_lineno Current line number in file being parsed.
 *
 *   alltok   Control flag for the kind of tokens you want (C or generic)
 *   dialect  Control flag to change the preprocessor for Ansi C.
 *
 * TODO:
 *    #asm -> asm("...") translation.
 *    ?: in #if expressions
 *    Complete #line directive.
 *    \n in "\n" in a stringized argument.
 *    Comments in stringized arguments should be deleted.
 *
 *    Poss: Seperate current directory for #include from errors (#line).
 *          (For editors that hunt down source files)
 *    Poss: C99 Variable macro args.
 */

#define KEEP_SPACE	0
#define SKIP_SPACE	1

#define EOT	4
#define SYN	22

char curword[WORDSIZE];
int alltok = 0;
int dialect = 0;

FILE *curfile;
char *c_fname;
int c_lineno = 0;

typedef long int_type;		/* Used for preprocessor expressions */
static int curtok = 0;		/* Used for preprocessor expressions */

static int fi_count = 0;
static FILE *saved_files[MAX_INCLUDE];
static char *saved_fname[MAX_INCLUDE];
static int saved_lines[MAX_INCLUDE];

static char *def_ptr = 0;
static char *def_start = 0;
static struct define_item *def_ref = 0;

static int def_count = 0;
static char *saved_def[MAX_DEFINE];
static char *saved_start[MAX_DEFINE];
static long saved_unputc[MAX_DEFINE];
static struct define_item *saved_ref[MAX_DEFINE];

static long unputc = 0;

static int last_char = '\n';
static int in_preproc = 0;
static int dont_subst = 0;
static int quoted_str = 0;

static int if_count = 0;
static int if_false = 0;
static int if_has_else = 0;
static int if_hidden = 0;
static unsigned int if_stack = 0;

struct arg_store {
	char *name;
	char *value;
	int in_define;
};

static int chget(void);
static int chget_raw(void);
static void unchget(int);
static int gettok_nosub(void);
static int get_onetok(int);
static int pgetc(void);
static int do_preproc(void);
static int do_proc_copy_hashline(void);
static int do_proc_if(int);
static void do_proc_include(void);
static void do_proc_define(void);
static void do_proc_undef(void);
static void do_proc_else(void);
static void do_proc_endif(void);
static void do_proc_tail(void);
static int get_if_expression(void);
static int_type get_expression(int);
static int_type get_exp_value(void);
static void gen_substrings(char *, char *, int, int);
static char *insert_substrings(char *, struct arg_store *, int);

int gettok(void)
{
	int ch;

	for (;;) {
		/* Tokenised C-Preprocessing */
		if (!quoted_str) {
			if (alltok)
				ch = get_onetok(KEEP_SPACE);
			else
				ch = get_onetok(SKIP_SPACE);

			if (ch == '"' || ch == '\'')
				quoted_str = ch;

			if (ch == TK_WORD) {
				struct token_trans *p = is_ckey(curword, strlen(curword));
				if (p)
					return p->token;
			}

			if (ch == '\n')
				continue;
			return ch;
		}

		/* Special for quoted strings */
		*curword = '\0';
		ch = chget();
		if (ch == EOF)
			return ch;

		*curword = ch;
		curword[1] = '\0';

		if (ch == quoted_str) {
			if (ch == '"') {
				if (dialect == DI_ANSI) {
					/* Found a terminator '"' check for ansi continuation */
					while ((ch = pgetc()) <= ' ' && ch != EOF);
					if (ch == '"')
						continue;
					unchget(ch);
					*curword = '"';
					curword[1] = '\0';
				}

				quoted_str = 0;
				return '"';
			} else {
				quoted_str = 0;
				return ch;
			}
		}
		if (ch == '\n') {
			quoted_str = 0;
			unchget(ch);	/* Make sure error line is right */
			return ch;
		}
		if (ch == '\\') {
			unchget(ch);
			ch = get_onetok(KEEP_SPACE);
			return ch;
		}
		return TK_STR;
	}
}

static int gettok_nosub(void)
{
	int rv;
	dont_subst++;
	rv = get_onetok(SKIP_SPACE);
	dont_subst--;
	return rv;
}

static int get_onetok(int keep)
{
	char *p;
	int state;
	register int ch, cc;

      Try_again:
	*(p = curword) = '\0';
	state = cc = ch = 0;

	/* First skip whitespace, if the arg says so then we need to keep it */
	while ((ch = pgetc()) == ' ' || ch == '\t') {
		if (keep == KEEP_SPACE) {
			if (p < curword + WORDSIZE - 1) {
				*p++ = ch;	/* Clip to WORDSIZE */
				*p = '\0';
			}
		}
	}

	if (ch > 0xFF)
		return ch;
	if (p != curword) {
		unchget(ch);
		return TK_WSPACE;
	}
	if (ch == '\n')
		return ch;
	if (ch == EOF)
		return ch;
	if (ch >= 0 && ch < ' ')
		goto Try_again;

	for (;;) {
		switch (state) {
		case 0:
			if ((ch >= 'A' && ch <= 'Z')
			    || (ch >= 'a' && ch <= 'z')
			    || ch == '_' || ch == '$' || ch >= 0x80)
				state = 1;
			else if (ch == '0')
				state = 2;
			else if (ch >= '1' && ch <= '9')
				state = 5;
			else
				goto break_break;
			break;
		case 1:
			if ((ch >= '0' && ch <= '9')
			    || (ch >= 'A' && ch <= 'Z')
			    || (ch >= 'a' && ch <= 'z')
			    || ch == '_' || ch == '$' || ch >= 0x80)
				break;
			else
				goto break_break;
		case 2:
			if (ch >= '0' && ch <= '7')
				state = 3;
			else if (ch == 'x' || ch == 'X')
				state = 4;
			else
				goto break_break;
			break;
		case 3:
			if (ch >= '0' && ch <= '7')
				break;
			else
				goto break_break;
		case 4:
			if ((ch >= '0' && ch <= '9')
			    || (ch >= 'A' && ch <= 'F')
			    || (ch >= 'a' && ch <= 'f'))
				break;
			else
				goto break_break;
		case 5:
		case 6:
			if (ch >= '0' && ch <= '9');
			else if (ch == '.' && state != 6)
				state = 6;
			else if (ch == 'e' || ch == 'E')
				state = 7;
			else
				goto break_break;
			break;
		case 7:
			if (ch == '+' || ch == '-')
				break;
			state = 8;
			/* FALLTHROUGH */
		case 8:
			if (ch >= '0' && ch <= '9')
				break;
			else
				goto break_break;
		}
		if (cc < WORDSIZE - 1)
			*p++ = ch;	/* Clip to WORDSIZE */
		*p = '\0';
		cc++;
		ch = chget();
		if (ch == SYN)
			ch = chget();
	}
      break_break:
	/* Numbers */
	if (state >= 2) {
		if (state < 6) {
			if (ch == 'u' || ch == 'U') {
				if (cc < WORDSIZE - 1)
					*p++ = ch;	/* Clip to WORDSIZE */
				*p = '\0';
				cc++;
				ch = chget();
			}
			if (ch == 'l' || ch == 'L') {
				if (cc < WORDSIZE - 1)
					*p++ = ch;	/* Clip to WORDSIZE */
				*p = '\0';
				cc++;
			} else
				unchget(ch);
			return TK_NUM;
		}
		unchget(ch);
		return TK_FLT;
	}

	/* Words */
	if (state == 1) {
		struct define_item *ptr = NULL;
		unchget(ch);
		if (!dont_subst && (ptr = read_entry(curword)) != 0 && !(ptr->flags & F_INUSE)) {
			if (def_count >= MAX_DEFINE) {
				cwarn("Preprocessor recursion overflow");
				unlock_entry(ptr);
				return TK_WORD;
			} else if (ptr->arg_count >= 0) {
				/* An open bracket must follow the word */
				int ch1 = 0;
				while ((ch = chget()) == ' ' || ch == '\t')
					ch1 = ch;
				if (ch != '(') {
					unchget(ch);
					if (ch1)
						unchget(ch1);
					unlock_entry(ptr);
					return TK_WORD;
				}

				/* We have arguments to process so lets do so. */
				gen_substrings(ptr->name, ptr->value, ptr->arg_count, ptr->flags & F_VARARG);

				/* Don't mark macros with arguments as in use, it's very
				 * difficult to say what the correct result would be so
				 * I'm letting the error happen. Also if I do block
				 * recursion then it'll also block 'pseudo' recursion
				 * where the arguments have a call to this macro.
				 *
				 def_ref = ptr;
				 ptr->in_use = 1;
				 */
				unlock_entry(ptr);
			} else if (ptr->value[0]) {
				/* Simple direct substitution; note the shortcut (above) for
				 * macros that are defined as null */
				saved_ref[def_count] = def_ref;
				saved_def[def_count] = def_ptr;
				saved_start[def_count] = def_start;
				saved_unputc[def_count] = unputc;
				def_count++;
				unputc = 0;
				def_ref = ptr;
				def_ptr = ptr->value;
				def_start = 0;
				ptr->flags |= F_INUSE;
			}
			goto Try_again;
		}
		unlock_entry(ptr);
		return TK_WORD;
	}

	/* Quoted char for preprocessor expressions */
	if (in_preproc && ch == '\'') {
		*p++ = ch;
		ch = chget();
		for (;;) {
			if (cc < WORDSIZE - 1)
				*p++ = ch;	/* Clip to WORDSIZE */
			*p = '\0';
			cc++;
			if (ch == '\'' || ch == '\n')
				break;

			if (ch == '\\') {
				ch = chget();
				if (cc < WORDSIZE - 1)
					*p++ = ch;	/* Clip to WORDSIZE */
				*p = '\0';
				cc++;
			}
			ch = chget();
		}
		ch = TK_QUOT;
	}

	/* Collect and translate \xyx strings, (should probably translate these
	 * all to some standard form (eg \ooo plus \N )
	 *
	 * ___________________________________________________________________
	 * | new-line          NL (LF)   \n|  audible alert        BEL   \a  |
	 * | horizontal tab    HT        \t|  question mark        ?     \?  |
	 * | vertical tab      VT        \v|  double quote         "     \"  |
	 * | backspace         BS        \b|  octal escape         ooo   \ooo|
	 * | carriage return   CR        \r|  hexadecimal escape   hh    \xhh|
	 * | formfeed          FF        \f|  backslash            \     \\  |
	 * | single quote      '         \'|                                 |
	 * |_______________________________|_________________________________|
	 */

	if (ch == '\\') {
		int i;

		*p++ = ch;
		ch = chget();
		if (ch >= '0' && ch <= '7') {
			for (i = 0; i < 3; i++) {
				if (ch >= '0' && ch <= '7') {
					*p++ = ch;
					ch = chget();
				}
			}
			unchget(ch);
		} else if (ch == 'x' || ch == 'X') {
			*p++ = ch;
			ch = chget();
			for (i = 0; i < 2; i++) {
				if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
					*p++ = ch;
					ch = chget();
				}
			}
			unchget(ch);
		} else if (ch == '?') {
			p[-1] = '?';
		} else if (ch != '\n' && ch != EOF) {
			*p++ = ch;
		} else
			unchget(ch);
		*p = '\0';
		return TK_STR;
	}

	/* Possible composite tokens */
	if (ch > ' ' && ch <= '~') {
		struct token_trans *tt;
		*curword = cc = ch;

		for (state = 1;; state++) {
			curword[state] = ch = chget();
			if (!(tt = is_ctok(curword, state + 1))) {
				unchget(ch);
				curword[state] = '\0';
				return cc;
			}
			cc = tt->token;
		}
	}
	return ch;
}

static int pgetc(void)
{
	register int ch, ch1;

	for (;;) {
		if ((ch = chget()) == EOF)
			return ch;

		if (!in_preproc && last_char == '\n' && ch == '#') {
			in_preproc = 1;
			ch = do_preproc();
			in_preproc = 0;
			if (if_false || ch == 0)
				continue;

			last_char = '\n';
			return ch;
		}
		if (last_char != '\n' || (ch != ' ' && ch != '\t'))
			last_char = ch;

		/* Remove comments ... */
		if (ch != '/') {
			if (if_false && !in_preproc)
				continue;
			return ch;
		}
		ch1 = chget();	/* Allow "/\\\n*" as comment start too!? */

		if (ch1 == '/') {	/* Double slash style comments */
			do {
				ch = chget();
			} while (ch != '\n' && ch != EOF);
			return ch;	/* Keep the return. */
		}

		if (ch1 != '*') {
			unchget(ch1);
			if (if_false && !in_preproc)
				continue;
			return ch;
		}

		for (;;) {
			if (ch == '*') {
				ch = chget();
				if (ch == EOF)
					return EOF;
				if (ch == '/')
					break;
			} else
				ch = chget();
		}
		if (dialect == DI_ANSI)
			return ' ';	/* If comments become " " */
		else
			return SYN;	/* Comments become nulls, but we need a
					 * marker so I can do token concat properly. */
	}
}

/* This function handles the first and second translation phases of Ansi-C */
static int chget(void)
{
	register int ch1;
	int ch;

	for (;;) {
		ch = chget_raw();
		if (ch == '\\') {
			ch1 = chget_raw();
			if (ch1 == '\n')
				continue;
			unchget(ch1);
		}

		/* Ansi trigraphs -- Ewww, it needs lots of 'unchget' space too. */
		if (dialect == DI_ANSI && ch == '?') {
			ch1 = chget_raw();
			if (ch1 != '?')
				unchget(ch1);
			else {
				static char trig1[] = "()<>/!'-=";
				static char trig2[] = "[]{}\\|^~#";
				char *s;
				ch1 = chget_raw();
				s = strchr(trig1, ch1);
				if (s) {
					unchget(trig2[s - trig1]);	/* Unchget so that ??/ can be used as */
					continue;	/* a real backslash at EOL. */
				} else {
					unchget(ch1);
					unchget('?');
				}
			}
		}

		return ch;
	}
}

static void unchget(int ch)
{
#if CPP_DEBUG
	fprintf(stderr, "\b", ch);
#endif
	if (ch == 0)
		return;		/* Hummm */
	if (ch == EOF)
		ch = EOT;	/* EOF is pushed back as a normal character. */
	ch &= 0xFF;

	if (unputc & 0xFF000000)
		cerror("Internal character pushback stack overflow");
	else
		unputc = (unputc << 8) + (ch);
	if (ch == '\n')
		c_lineno--;
}

static int chget_raw(void)
#if CPP_DEBUG
{
	int ch;
	static int last_def = 0;
	static int last_fi = 0;
	if (last_fi != fi_count)
		fprintf(stderr, "<INC%d>", fi_count);
	if (last_def != def_count)
		fprintf(stderr, "<DEF%d>", def_count);
	last_def = def_count;
	last_fi = fi_count;

	ch = realchget();
	if (ch == EOF)
		fprintf(stderr, "<EOF>");
	else
		fprintf(stderr, "%c", ch);

	if (last_def != def_count)
		fprintf(stderr, "<DEF%d>", def_count);
	if (last_fi != fi_count)
		fprintf(stderr, "<INC%d>", fi_count);
	last_def = def_count;
	last_fi = fi_count;

	return ch;
}

static int realchget(void)
#endif
{
	int ch;
	for (;;) {
		if (unputc) {
			if ((unputc & 0xFF) == EOT && in_preproc)
				return '\n';
			ch = (unputc & 0xFF);
			unputc >>= 8;
			if (ch == EOT)
				ch = EOF;
			if (ch == '\n')
				c_lineno++;
			return ch;
		}

		if (def_ptr) {
			ch = *def_ptr++;
			if (ch)
				return (unsigned char) ch;
			if (def_start)
				free(def_start);
			if (def_ref) {
				def_ref->flags &= ~F_INUSE;
				unlock_entry(def_ref);
			}

			def_count--;
			def_ref = saved_ref[def_count];
			def_ptr = saved_def[def_count];
			def_start = saved_start[def_count];
			unputc = saved_unputc[def_count];
			continue;
		}

		ch = getc(curfile);
		if (ch == EOF && fi_count != 0) {
			fclose(curfile);
			fi_count--;
			curfile = saved_files[fi_count];
			if (c_fname)
				free(c_fname);
			c_fname = saved_fname[fi_count];
			c_lineno = saved_lines[fi_count];
			ch = '\n';	/* Ensure end of line on end of file */
		} else if (ch == '\n')
			c_lineno++;

		/* Treat all control characters, except the standard whitespace
		 * characters of TAB and NL as completely invisible.
		 */
		if (ch >= 0 && ch < ' ' && ch != '\n' && ch != '\t' && ch != EOF)
			continue;

		if (ch == EOF) {
			unchget(ch);
			return '\n';
		}		/* Ensure EOL before EOF */
		return (unsigned char) ch;
	}
}

static int do_preproc(void)
{
	int val, no_match = 0;

	if ((val = get_onetok(SKIP_SPACE)) == TK_WORD) {
		if (strcmp(curword, "ifdef") == 0)
			do_proc_if(0);
		else if (strcmp(curword, "ifndef") == 0)
			do_proc_if(1);
		else if (strcmp(curword, "if") == 0)
			do_proc_if(2);
		else if (strcmp(curword, "elif") == 0)
			do_proc_if(3);
		else if (strcmp(curword, "else") == 0)
			do_proc_else();
		else if (strcmp(curword, "endif") == 0)
			do_proc_endif();
		else if (if_false)
			no_match = 1;
		else {
			if (strcmp(curword, "include") == 0)
				do_proc_include();
			else if (strcmp(curword, "define") == 0)
				do_proc_define();
			else if (strcmp(curword, "undef") == 0)
				do_proc_undef();
			else if (strcmp(curword, "error") == 0) {
				strcpy(curword, "#error");
				do_proc_copy_hashline();
				pgetc();
				cerror(curword);
			} else if (strcmp(curword, "warning") == 0) {
				strcpy(curword, "#warning");
				do_proc_copy_hashline();
				pgetc();
				cwarn(curword);
			} else if (strcmp(curword, "pragma") == 0) {
				do_proc_copy_hashline();
				pgetc();
				/* Ignore #pragma ? */
			} else if (strcmp(curword, "line") == 0) {
				do_proc_copy_hashline();
				pgetc();
				/* Ignore #line for now. */
			} else if (strcmp(curword, "asm") == 0) {
				alltok |= 0x100;
				return do_proc_copy_hashline();
			} else if (strcmp(curword, "endasm") == 0) {
				alltok &= ~0x100;
				return do_proc_copy_hashline();
			} else
				no_match = 1;
		}
	} else if (!val) {
		/* Empty directives used to denote that a file is to be run through
		 * the preprocessor in K&R. Do not complain if we got no token. */
		no_match = 1;
	}

	if (no_match) {
		if (!if_false)
			cerror("Unknown preprocessor directive");
		while (val != '\n')
			val = pgetc();
	}

	*curword = 0;		/* Just in case */
	return 0;
}

static int do_proc_copy_hashline(void)
{
	int off, ch;

	off = strlen(curword);

	while ((ch = pgetc()) != '\n') {
		if (off < WORDSIZE)
			curword[off++] = ch;
	}
	if (off == WORDSIZE) {
		cerror("Preprocessor directive too long");
		curword[WORDSIZE - 1] = '\0';
	} else
		curword[off] = '\0';

	unchget('\n');
	return TK_COPY;
}

static void do_proc_include(void)
{
	int ch, ch1;
	char *p;
	FILE *fd;

	ch = get_onetok(SKIP_SPACE);
	if (ch == '<' || ch == '"') {
		if (ch == '"')
			ch1 = ch;
		else
			ch1 = '>';
		p = curword;
		while (p < curword + WORDSIZE - 1) {
			ch = pgetc();
			if (ch == '\n')
				break;
			if (ch == ch1) {
				*p = '\0';
				p = xstrdup(curword);

				do {
					ch1 = pgetc();
				} while (ch1 == ' ' || ch1 == '\t');
				unchget(ch1);
				do_proc_tail();

				saved_files[fi_count] = curfile;
				saved_fname[fi_count] = c_fname;
				saved_lines[fi_count] = c_lineno;

				fd = open_include(p, "r", (ch == '"'));
				if (fd) {
					fi_count++;
					curfile = fd;
				} else {
					fputc('\'', stderr);
					fputs(p, stderr);
					fputs("' - ", stderr);
					cerror("Cannot open include file");
				}

				return;
			}
			*p++ = ch;
		}
	}
	cerror("Bad #include command");
	while (ch != '\n')
		ch = pgetc();
	return;
}

static void mem(void)
{
	cfatal("Preprocessor out of memory");
}

void *xmalloc(size_t size)
{
	register void *p;
	do {
		p = malloc(size);
		if (p)
			return p;
	} while(memory_short());
	mem();
}

void *xrealloc(void *ptr, size_t size)
{
	register void *p;
	do {
		p = realloc(ptr, size);
		if (p)
			return p;
	} while(memory_short());
	mem();
}

char *xstrdup(const char *ptr)
{
	register char *p;
	do {
		p = strdup(ptr);
		if (p)
			return p;
	} while(memory_short());
	mem();
}

static void do_proc_define(void)
{
	int ch, ch1;
	struct define_item *ptr, *old_value = NULL;
	int cc, len;
	char name[WORDSIZE];

	if ((ch = gettok_nosub()) == TK_WORD) {
		strcpy(name, curword);
		ptr = read_entry(name);
		if (ptr) {
			set_entry(name, NULL, 0);	/* Unset var */
			if (ptr->flags & F_INUSE)
				/* Eeeek! This shouldn't happen; so just let it leak. */
				cwarn("macro redefined while it was in use!?");
			old_value = ptr;
		}

		/* Skip blanks */
		for (ch = ch1 = pgetc(); ch == ' ' || ch == '\t'; ch = pgetc());

		len = WORDSIZE;
		ptr = xmalloc(sizeof(struct define_item) + WORDSIZE);
		ptr->value[cc = 0] = '\0';

		/* Add in arguments */
		if (ch1 == '(') {
			ptr->arg_count = 0;
			for (;;) {
				ch = gettok_nosub();
				if (ptr->arg_count == 0 && ch == ')')
					break;
				if (ch == TK_WORD) {
					if (cc + strlen(curword) + 4 >= len) {
						len = cc + WORDSIZE;
						ptr = (struct define_item *) xrealloc(ptr, sizeof(struct define_item) + len);
					}
					if (cc + strlen(curword) < len) {
						strcpy(ptr->value + cc, curword);
						cc += strlen(curword);
						strcpy(ptr->value + cc, ",");
						cc++;
						ptr->arg_count++;
						ch = gettok_nosub();
						if (ch == TK_ELLIPSIS) {
							ptr->flags |= F_VARARG;
							ch = gettok_nosub();
							if (ch == ',')
								ch = '*';	/* Force error if not ')' */
						}
						if (ch == ')')
							break;
						if (ch == ',')
							continue;
					}
				}
				cerror("Bad #define command");
				free(ptr);
				while (ch != '\n')
					ch = pgetc();
				set_entry(name, NULL, 0);	/* Kill var */
				unlock_entry(old_value);
				return;
			}
			while ((ch = pgetc()) == ' ' || ch == '\t');
		} else
			ptr->arg_count = -1;

		/* And the substitution string */
		while (ch != '\n') {
			if (cc + 4 > len) {
				len = cc + WORDSIZE;
				ptr = (struct define_item *) xrealloc(ptr, sizeof(struct define_item) + len);
			}
			ptr->value[cc++] = ch;
			ch = pgetc();
		}
		if (cc)
			ptr->value[cc++] = ' ';	/* Byte of lookahead for recursive macros */
		ptr->value[cc++] = '\0';

#if CPP_DEBUG
		if (cc == 1)
			fprintf(stderr, "\n### Define '%s' as null\n", name);
		else if (ptr->arg_count < 0)
			fprintf(stderr, "\n### Define '%s' as '%s'\n", name, ptr->value);
		else
			fprintf(stderr, "\n### Define '%s' as %d args '%s'\n", name, ptr->arg_count, ptr->value);
#endif

		/* Clip to correct size and save */
		ptr = (struct define_item *) xrealloc(ptr, sizeof(struct define_item) + cc);
		ptr->name = set_entry(name, ptr, sizeof(struct define_item) + cc);
		ptr->flags &= ~F_INUSE;

		if (old_value) {
			if (strcmp(old_value->value, ptr->value) != 0)
				cwarn("#define redefined macro");
			free(old_value);
		}
	} else
		cerror("Bad #define command");
	while (ch != '\n')
		ch = pgetc();
}

static void do_proc_undef(void)
{
	int ch;
	struct define_item *ptr;
	if ((ch = gettok_nosub()) == TK_WORD) {
		ptr = read_entry(curword);
		if (ptr) {
			set_entry(curword, NULL, 0);	/* Unset var */
			if (ptr->flags & F_INUSE)
				/* Eeeek! This shouldn't happen; so just let it leak. */
				cwarn("macro undefined while it was in use!?");
			else
				free(ptr);
		}
		do_proc_tail();
	} else {
		cerror("Bad #undef command");
		while (ch != '\n')
			ch = pgetc();
	}
}

static int do_proc_if(int type)
{
	int ch = 0;
	struct define_item *ptr;
	if (if_false && if_hidden) {
		if (type != 3)
			if_hidden++;
		do_proc_tail();
		return 0;
	}

	if (type == 3) {
		if (if_count == 0)
			cerror("#elif without matching #if");
		else {
			if (if_has_else)
				cerror("#elif following #else for one #if");
			if (if_has_else || if_false != 1) {
				if_false = 2;
				while (ch != '\n')
					ch = pgetc();
				return 0;
			}
			if_false = 0;
		}
		if_has_else = 0;
	}
	if (if_false) {
		if (type != 3)
			if_hidden++;
		do_proc_tail();
	} else {
		if (type != 3) {
			if_count++;
			if_stack <<= 1;
			if_stack |= if_has_else;
			if_has_else = 0;
		}
		if (type > 1) {
			ch = get_if_expression();
			if_false = !ch;
		} else {
			ch = gettok_nosub();
			if (ch == TK_WORD) {
				do_proc_tail();
				ptr = read_entry(curword);
				if_false = (ptr == NULL);
				unlock_entry(ptr);
				if (type == 1)
					if_false = !if_false;
			} else {
				cerror("Bad #if command");
				if_false = 0;
				while (ch != '\n')
					ch = pgetc();
			}
		}
	}
	return 0;
}

static void do_proc_else(void)
{
	if (if_hidden == 0) {
		if (if_count == 0)
			cerror("#else without matching #if");
		else
			if_false = (if_false ^ 1);
		if (if_has_else)
			cerror("Multiple #else's for one #if");
		if_has_else = 1;
	}
	do_proc_tail();
}

static void do_proc_endif(void)
{
	if (if_hidden)
		if_hidden--;
	else {
		if (if_count == 0)
			cerror("Unmatched #endif");
		else {
			if_count--;
			if_false = 0;
			if_has_else = (if_stack & 1);
			if_stack >>= 1;
		}
	}
	do_proc_tail();
}

static void do_proc_tail(void)
{
	int ch, flg = 1;
	while ((ch = pgetc()) != '\n')
		if (ch > ' ') {
			if (!if_false && flg)
				cwarn("Unexpected text following preprocessor command");
			flg = 0;
		}
}

static int get_if_expression(void)
{
	int value = get_expression(0);

	if (curtok != '\n')
		do_proc_tail();

	return value;
}

static int_type get_expression(int prio)
{
	int_type lvalue;
	int_type rvalue;
	int no_op = 0;

	curtok = get_onetok(SKIP_SPACE);
	lvalue = get_exp_value();

	do {
		switch (curtok) {
		case '*':
		case '/':
		case '%':
			if (prio >= 10)
				return lvalue;
			break;
		case '+':
		case '-':
			if (prio >= 9)
				return lvalue;
			break;
		case TK_RIGHT_OP:
		case TK_LEFT_OP:
			if (prio >= 8)
				return lvalue;
			break;
		case '<':
		case '>':
		case TK_LE_OP:
		case TK_GE_OP:
			if (prio >= 7)
				return lvalue;
			break;
		case TK_EQ_OP:
		case TK_NE_OP:
			if (prio >= 6)
				return lvalue;
			break;
		case '&':
			if (prio >= 5)
				return lvalue;
			break;
		case '^':
			if (prio >= 4)
				return lvalue;
			break;
		case '|':
			if (prio >= 3)
				return lvalue;
			break;
		case TK_AND_OP:
			if (prio >= 2)
				return lvalue;
			break;
		case TK_OR_OP:
			if (prio >= 1)
				return lvalue;
			break;
		}
		switch (curtok) {
		case '*':
			rvalue = get_expression(10);
			lvalue *= rvalue;
			break;
		case '/':
			rvalue = get_expression(10);
			if (rvalue)
				lvalue /= rvalue;
			break;
		case '%':
			rvalue = get_expression(10);
			if (rvalue)
				lvalue %= rvalue;
			break;
		case '+':
			rvalue = get_expression(9);
			lvalue += rvalue;
			break;
		case '-':
			rvalue = get_expression(9);
			lvalue -= rvalue;
			break;
		case TK_RIGHT_OP:
			rvalue = get_expression(8);
			lvalue >>= rvalue;
			break;
		case TK_LEFT_OP:
			rvalue = get_expression(8);
			lvalue <<= rvalue;
			break;
		case '<':
			rvalue = get_expression(7);
			lvalue = (lvalue < rvalue);
			break;
		case '>':
			rvalue = get_expression(7);
			lvalue = (lvalue > rvalue);
			break;
		case TK_LE_OP:
			rvalue = get_expression(7);
			lvalue = (lvalue <= rvalue);
			break;
		case TK_GE_OP:
			rvalue = get_expression(7);
			lvalue = (lvalue >= rvalue);
			break;
		case TK_EQ_OP:
			rvalue = get_expression(6);
			lvalue = (lvalue == rvalue);
			break;
		case TK_NE_OP:
			rvalue = get_expression(6);
			lvalue = (lvalue != rvalue);
			break;
		case '&':
			rvalue = get_expression(5);
			lvalue = (lvalue & rvalue);
			break;
		case '^':
			rvalue = get_expression(4);
			lvalue = (lvalue ^ rvalue);
			break;
		case '|':
			rvalue = get_expression(3);
			lvalue = (lvalue | rvalue);
			break;
		case TK_AND_OP:
			rvalue = get_expression(2);
			lvalue = (lvalue && rvalue);
			break;
		case TK_OR_OP:
			rvalue = get_expression(1);
			lvalue = (lvalue || rvalue);
			break;

		case '?':	/* XXX: To add */

		default:
			no_op = 1;
		}
	}
	while (prio == 0 && !no_op);

	return lvalue;
}

static int_type get_exp_value(void)
{
	int_type value = 0;
	int sign = 1;
	struct define_item *ptr;

	if (curtok == '!') {
		curtok = get_onetok(SKIP_SPACE);
		return !get_exp_value();
	}
	if (curtok == '~') {
		curtok = get_onetok(SKIP_SPACE);
		return ~get_exp_value();
	}

	while (curtok == '+' || curtok == '-') {
		if (curtok == '-')
			sign = -sign;
		curtok = get_onetok(SKIP_SPACE);
	}

	if (curtok == TK_NUM) {
		value = strtoul(curword, (void *) 0, 0);
		curtok = get_onetok(SKIP_SPACE);
	} else if (curtok == TK_QUOT) {
		value = curword[1];
		if (value == '\\') {
			if (curword[2] >= '0' && curword[2] <= '7') {
				value = curword[2] - '0';
				if (curword[3] >= '0' && curword[3] <= '7') {
					value = (value << 3) + curword[3] - '0';
					if (curword[4] >= '0' && curword[4] <= '7') {
						value = (value << 3) + curword[4] - '0';
					}
				}
			} else
				switch (curword[2]) {
				case 'n':
					value = '\n';
					break;
				case 'f':
					value = '\f';
					break;
				case 't':
					value = '\t';
					break;
				default:
					value = curword[2];
					break;
				}
		}
#ifdef NATIVE_CPP
		value = (char) value;	/* Fix range */
#elif SIGNED_CHAR
		value = (signed char) value;
#else
		value = (unsigned char) value;
#endif
		curtok = get_onetok(SKIP_SPACE);
	} else if (curtok == TK_WORD) {
		value = 0;
		if (strcmp("defined", curword) == 0) {
			curtok = gettok_nosub();
			if (curtok == '(' && gettok_nosub() != TK_WORD)
				cerror("'defined' keyword requires argument");
			else {
				ptr = read_entry(curword);
				value = (ptr != 0);
				unlock_entry(ptr);
				if (curtok == '(' && gettok_nosub() != ')')
					cerror("'defined' keyword requires closing ')'");
				else
					curtok = get_onetok(SKIP_SPACE);
			}
		} else
			curtok = get_onetok(SKIP_SPACE);

	} else if (curtok == '(') {
		value = get_expression(0);
		if (curtok == ')')
			curtok = get_onetok(SKIP_SPACE);
		else {
			curtok = '$';
			cerror("Expected ')'");
		}
	}

	return sign < 0 ? -value : value;
}

static void gen_substrings(char *macname, char *data_str, int arg_count, int is_vararg)
{
	char *mac_text = 0;
	struct arg_store *arg_list;
	int ac, ch, cc, len;

	int paren_count = 0;
	int in_quote = 0;
	int quote_char = 0;
	int commas_found = 0;
	int args_found = 0;

	arg_list = xmalloc(sizeof(struct arg_store) * arg_count);
	memset(arg_list, 0, sizeof(struct arg_store) * arg_count);

	for (ac = 0; *data_str && ac < arg_count; data_str++) {
		if (*data_str == ',') {
			ac++;
			continue;
		}

		if (arg_list[ac].name == 0)
			cc = len = 0;

		if (cc + 2 >= len) {
			len += 20;
			arg_list[ac].name = xrealloc(arg_list[ac].name, len);
		}
		arg_list[ac].name[cc++] = *data_str;
		arg_list[ac].name[cc] = '\0';
	}

	for (;;) {
		if ((ch = chget()) == EOF)
			break;
		if (in_quote == 2) {
			in_quote = 1;
		} else if (in_quote) {
			if (ch == quote_char)
				in_quote = 0;
			if (ch == '\\')
				in_quote = 2;
		} else {
			if (ch == '(')
				paren_count++;
			if (ch == '"' || ch == '\'') {
				in_quote = 1;
				quote_char = ch;
			}
			if (paren_count == 0 && ch == ',') {
				commas_found++;
				if (commas_found < arg_count)
					continue;
			}
			if (ch == ')') {
				if (paren_count == 0)
					break;
				paren_count--;
			}
		}
		args_found = 1;
		/* Too many args, deal with, or ignore, the rest. */
		if (commas_found >= arg_count) {
			if (arg_count == 0)
				continue;
			ac = arg_count - 1;
		} else
			ac = commas_found;

		if (arg_list[ac].value == 0) {
			cc = len = 0;
			arg_list[ac].in_define = def_count;
		}

		if (cc + 2 >= len) {
			len += 20;
			arg_list[ac].value = xrealloc(arg_list[ac].value, len);
		}

#if 0
		if (ch == '\n' && cc > 0 && arg_list[ac].value[cc - 1] == '\n') {
		... ?}
#endif

		arg_list[ac].value[cc++] = ch;
		arg_list[ac].value[cc] = '\0';
	}

	if (commas_found || args_found)
		args_found = commas_found + 1;

	if (arg_count == 0 && args_found != 0)
		cerror("Arguments given to macro without them.");
	else if (!is_vararg && arg_count != args_found)
		cwarn("Incorrect number of macro arguments");

	mac_text = insert_substrings(data_str, arg_list, arg_count);

	/* 
	 * At this point 'mac_text' contains the full expansion of the macro.
	 *
	 * So we could scan this for calls to this macro and if we find one
	 * that _exactly_ matches this call (including arguments) then we mark
	 * this call's in_use flag.
	 *
	 * OTOH, it would probably be best to throw away this expansion and
	 * pretend we never noticed this macro expansion in the first place.
	 *
	 * Still this is mostly academic as the error trapping works and
	 * recursive macros _with_arguments_ are both rare and unpredictable.
	 */

	if (arg_list) {
		for (ac = 0; ac < arg_count; ac++) {
			if (arg_list[ac].name)
				free(arg_list[ac].name);
			if (arg_list[ac].value)
				free(arg_list[ac].value);
		}
		free(arg_list);
	}

	saved_ref[def_count] = def_ref;
	saved_def[def_count] = def_ptr;
	saved_start[def_count] = def_start;
	saved_unputc[def_count] = unputc;
	def_count++;
	unputc = 0;
	def_ptr = mac_text;
	def_start = mac_text;
	def_ref = 0;
#if CPP_DEBUG
	fprintf(stderr, "\n### <DEF%d='%s'>\n", def_count, mac_text);
#endif
}

static char *insert_substrings(char *data_str, struct arg_store *arg_list, int arg_count)
{
	int ac, ch;
	char *p, *s;
	char *rv = 0;
	int len = 0;
	int cc = 0;
	int in_quote = 0;
	int quote_char = 0;
	int ansi_stringize = 0;

#if CPP_DEBUG
	fprintf(stderr, "\n### Macro substitution in '%s'\n", data_str);
	for (ac = 0; ac < arg_count; ac++) {
		fprintf(stderr, "### Argument %d (%s) = '%s'\n", ac + 1, arg_list[ac].name, arg_list[ac].value);
	}
#endif

	rv = xmalloc(4);
	*rv = '\0';
	len = 4;

	while (*data_str) {
		p = curword;

		if (dialect == DI_ANSI) {
			if (in_quote == 2)
				in_quote = 1;
			else if (in_quote) {
				if (*data_str == quote_char)
					in_quote = 0;
				if (*data_str == '\\')
					in_quote = 2;
			} else {
				if (*data_str == '"' || *data_str == '\'') {
					in_quote = 1;
					quote_char = *data_str;
				}
			}
		}

		if (!in_quote)
			for (;;) {
				ch = *data_str;
				if ((ch >= '0' && ch <= '9')
				    || (ch >= 'A' && ch <= 'Z')
				    || (ch >= 'a' && ch <= 'z')
				    || ch == '_' || ch == '$')
					*p++ = *data_str++;
				else
					break;
			}

		if (p == curword) {
			/* Ansi Stringize and concat */
			if (*data_str == '#' && dialect != DI_KNR) {
				if (data_str[1] == '#') {
					while (cc > 0 && (rv[cc - 1] == ' ' || rv[cc - 1] == '\t'))
						cc--;
					data_str += 2;
					while (*data_str == ' ' || *data_str == '\t')
						data_str++;
					if (*data_str == '\0') {	/* Hummm */
						data_str--;
						cerror("'##' operator at end of macro");
					}
					continue;
				}
				data_str++;
				ansi_stringize = 1;
				continue;
			}

			if (ansi_stringize) {
				ansi_stringize = 0;
				cerror("'#' operator should be followed by a macro argument name");
			}

			/* Other characters ... */
			if (cc + 2 > len) {
				len += 20;
				rv = xrealloc(rv, len);
			}
			rv[cc++] = *data_str++;
			continue;
		}
		*p = '\0';
		s = curword;
		for (ac = 0; ac < arg_count; ac++) {
			if (*curword == arg_list[ac].name[0] && strcmp(curword, arg_list[ac].name) == 0) {
				s = arg_list[ac].value;
				if (!s)
					s = "";
				else
					/* Ansi stringize operation, this is very messy! */
				if (ansi_stringize) {
					struct define_item *ptr = NULL;
					if (arg_list[ac].in_define) {
						if ((ptr = read_entry(s)) && ptr->arg_count == -1) {
							s = ptr->value;
						}
					}

					rv[cc++] = '"';
					while (*s == ' ' || *s == '\t')
						s++;
					while (*s) {
						if (cc + 4 > len) {
							len += 20;
							rv = xrealloc(rv, len);
						}
						if (*s == '"')
							rv[cc++] = '\\';
						rv[cc++] = *s++;
					}
					while (cc > 0 && (rv[cc - 1] == ' ' || rv[cc - 1] == '\t'))
						cc--;
					rv[cc++] = '"';
					rv[cc++] = '\0';
					ansi_stringize = 0;
					s = "";
					unlock_entry(ptr);
					break;
				}

				break;
			}
		}

		if (ansi_stringize) {
			ansi_stringize = 0;
			cerror("'#' operator should be followed by a macro argument name");
		}

		if (cc + 2 + strlen(s) > len) {
			len += strlen(s) + 20;
			rv = xrealloc(rv, len);
		}
		strcpy(rv + cc, s);
		cc = strlen(rv);
	}

	rv[cc] = '\0';
	return rv;
}
