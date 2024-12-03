
#include <stdio.h>
#if __STDC__
#include <stdlib.h>
#include <locale.h>
#else
#include <malloc.h>
#endif
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include "cc.h"

#define MAXINCPATH	5

int main(int argc, char *argv[]);
void undefine_macro(char *name);
void define_macro(char *name);
void print_toks_cpp(void);
void print_toks_raw(void);
void define_macro(char *);
void undefine_macro(char *);
void cmsg(char *mtype, char *str);
char *token_txn(int);
void pr_indent(int);
void hash_line(void);

#if defined(__linux__)
char *_ltoa(long v)
{
	static char buf[32];
	sprintf(buf, "%ld", v);
	return buf; 
}
#endif

char *include_paths[MAXINCPATH];

char last_name[256] = "";
int last_line = -1;
int debug_mode = 0;
int p_flag = 0;
int exit_code = 0;

char *outfile = 0;
FILE *ofd = 0;

int main(int argc, char *argv[])
{
	int ar, i;
	char *p;
	static char Usage[] = "Usage: cpp -E -Dxxx -Uxxx -Ixxx infile -o outfile";

#ifdef LC_CTYPE
	setlocale(LC_CTYPE, "");
#endif

	alltok = 1;		/* Get all tokens from the cpp. */

	for (ar = 1; ar < argc; ar++)
		if (argv[ar][0] == '-')
			switch (argv[ar][1]) {
			case 'd':
				debug_mode++;
				break;
			case 'E':
				break;
			case 'T':
				alltok = 0;
				break;
			case 'A':
				dialect = DI_ANSI;
				break;
			case 'K':
				dialect = DI_KNR;
				break;

			case 'P':
				p_flag++;
				break;

			case 'I':
				if (argv[ar][2])
					p = argv[ar] + 2;
				else {
					ar++;
					if (ar >= argc)
						cfatal(Usage);
					p = argv[ar];
				}
				for (i = 0; i < MAXINCPATH; i++)
					if (!include_paths[i]) {
						include_paths[i] = p;
						break;
					}
				if (i >= MAXINCPATH)
					cfatal("cpp: too many includes");
				break;
			case 'D':
				if (argv[ar][2])
					p = argv[ar] + 2;
				else {
					ar++;
					if (ar >= argc)
						cfatal(Usage);
					p = argv[ar];
				}
				define_macro(p);
				break;
			case 'U':
				if (argv[ar][2])
					p = argv[ar] + 2;
				else {
					ar++;
					if (ar >= argc)
						cfatal(Usage);
					p = argv[ar];
				}
				undefine_macro(p);
				break;
			case 'o':
				if (argv[ar][2])
					p = argv[ar] + 2;
				else {
					ar++;
					if (ar >= argc)
						cfatal(Usage);
					p = argv[ar];
				}
				if (outfile)
					cfatal(Usage);
				outfile = p;
				break;
			default:
				fputs("cpp: unknown option '", stderr);
				fputs(argv[ar], stderr);
				fputs("'\n", stderr);
				cfatal(Usage);
		} else if (!curfile) {
			/* Input file */
			curfile = fopen(argv[ar], "r");
			c_fname = argv[ar];
			c_lineno = 1;
			if (!curfile)
				cfatal("Cannot open input file");
		} else
			cfatal(Usage);

	if (!curfile)
		cfatal(Usage);
#if 0
	/* TODO: sort memory usage */		
	/* Define date and time macros. */
	if (dialect != DI_KNR) {
		time_t now;
		struct tm *tm;
		char *timep;
		char buf[32];
		time(&now);
		timep = ctime(&now);

		/* Yes, well */
		strcpy(buf, "__TIME__=\"");
		memcpy(buf + 9, timep + 11, 8);
		memcpy(buf + 17, "\"", 2);
		define_macro(buf);
		/* US order; Seems to be mandated by standard. */
		strcpy(buf, "__DATA__=\"XXX XX XXXX\"");
		memcpy(buf + 10, timep + 3, 3);
		memcpy(buf + 14, timep + 8, 2);
		memcpy(buf + 17, timep + 20, 4);
		define_macro(buf);
	}
#endif	

	if (outfile)
		ofd = fopen(outfile, "w");
	else
		ofd = stdout;
	if (!ofd)
		cfatal("Cannot open output file");

	hash_init();

#ifdef CPP_DEBUG
	if (debug_mode)
		print_toks_raw();
	else
#endif
		print_toks_cpp();

	if (outfile)
		fclose(ofd);
	exit(exit_code);
}

void undefine_macro(char *name)
{
	struct define_item *ptr;

	ptr = read_entry(name);
	if (ptr) {
		set_entry(name, NULL, 0);
		if (!(ptr->flags & F_INUSE))
			free(ptr);
	}
}

void define_macro(char *name)
{
	char *p;
	char *value;
	register struct define_item *ptr;
	unsigned size;

	if ((p = strchr(name, '=')) != 0) {
		*p = 0;
		value = p + 1;
	} else
		value = "1";

	undefine_macro(name);

	size = sizeof(struct define_item) + strlen(value);
	ptr = xmalloc(size);
	strcpy(ptr->value, value);
	ptr->arg_count = -1;
	ptr->flags = 0;
	ptr->name = set_entry(name, ptr, size);
}

extern int errno;

FILE *open_include(char *fname, char *mode, int checkrel)
{
	FILE *fd = 0;
	int i;
#ifdef __CC65__
	static
#endif
	char buf[256], *p;

	memory_short();
	if (checkrel) {
		strlcpy(buf, c_fname, 256);
		p = strrchr(buf, '/');
		if (p)
			*++p = 0;
		else
			*(p = buf) = 0;
		strlcpy(p, fname, p - buf);

		fd = fopen(buf, mode);
	}
	if (!fd) {
		for (i = 0; i < MAXINCPATH; i++)
			if (include_paths[i]) {
				strlcpy(buf, include_paths[i], 256);
				if (buf[strlen(buf) - 1] != '/')
					strlcat(buf, "/", 256);
				strlcat(buf, fname, 256);
				fd = fopen(buf, mode);
				if (fd)
					break;
				if (errno != ENOENT) {
					perror(buf);
					exit(1);
				}
			}
	}
	if (!fd)
		return fd;
	c_fname = xstrdup(buf);
	c_lineno = 1;

	return fd;
}

/*----------------------------------------------------------------------*/

static int outpos = 0;

void cmsg(char *mtype, char *str)
{
	if (c_fname && (*c_fname || c_lineno)) {
		fputs(c_fname, stderr);
		fputc(':', stderr);
		fputs(_ltoa(c_lineno), stderr);
		fputc(':', stderr);
	}

	if (mtype && *mtype) {
		fputs(mtype, stderr);
		fputs(": ", stderr);
	}
	fputs(str, stderr);
	fputc('\n', stderr);
}

void cfatal(char *str)
{
	cmsg("cpp: fatal: ", str);
	exit(255);
}

void cerror(char *str)
{
	exit_code = 1;
	cmsg("error", str);
}

void cwarn(char *str)
{
	cmsg("warning", str);
}

void pr_indent(int count)
{
	if (count > 10)
		count = 10;
	while (count > 0) {
		fputc('\t', ofd);
		count--;
	}
}

void hash_line(void)
{
	if (strcmp(last_name, c_fname) != 0)
		last_line = -1;
	if (c_lineno != last_line || last_line <= 0) {
		if (outpos != 0) {
			fputc('\n', ofd);
			outpos = 0;
			if (last_line > 0)
				last_line++;
		}
		while (c_lineno > last_line && (p_flag || c_lineno < last_line + 4) && last_line > 0 && !debug_mode) {
			fputc('\n', ofd);
			last_line++;
		}

		if (!p_flag && (c_lineno != last_line || last_line <= 0)) {
			fputs("# ", ofd);
			fputs(_ltoa(c_lineno), ofd);
			if (last_line <= 0) {
				fputs(" \"", ofd);
				fputs(c_fname, ofd);
				fputc('"', ofd);
			}
			fputc('\n', ofd);
		}

		strcpy(last_name, c_fname);
		last_line = c_lineno;
	}
}

static unsigned outstr(const char *p)
{
	fputs(p, ofd);
	return strlen(p);
}

void print_toks_cpp(void)
{
	int i;
	int indent = 0;
	int paren = 0;

	hash_line();
	while ((i = gettok()) != EOF) {
		hash_line();
		switch (i) {
		case '\n':
			cwarn("newline received from tokeniser!");
			break;

		case TK_STR:
			outpos += outstr(curword);
			break;

		case TK_COPY:
			if (outpos) {
				fputc('\n', ofd);
				last_line++;
			}
			outpos = 0;
			last_line++;
			fputc('#', ofd);
			fputs(curword, ofd);
			fputc('\n', ofd);
			break;

		case TK_FILE:
			*curword = '"';
			strcpy(curword + 1, c_fname);
			strcat(curword, "\"");
			if (0) {
		case TK_LINE:
				strcpy(curword, _ltoa(c_lineno));
			}
		/*FALLTHROUGH*/ default:
			if (!alltok) {
				if (i == '}' || i == TK_CASE || i == TK_DEFAULT)
					indent--;
				if (i == ')')
					paren--;

				if (outpos) {
					fputc(' ', ofd);
					outpos++;
				} else
					pr_indent(indent + (paren != 0));

				if (i == '{' || i == TK_CASE || i == TK_DEFAULT)
					indent++;
				if (i == ';')
					paren = 0;
				if (i == '(')
					paren++;
			}

			outpos += outstr(curword);

			if (i == '"' || i == '\'') {
				while ((i = gettok()) == TK_STR) {
					outpos += outstr(curword);
				}
				if (i != '\n')
					outpos += outstr(curword);
			}
			break;
		}
	}
	if (outpos)
		fputc('\n', ofd);
	outpos = 0;
}

#ifdef CPP_DEBUG
void print_toks_raw(void)
{
	int i;
	long val;

	hash_line();
	while ((i = gettok()) != EOF) {
		hash_line();
		switch (i) {
		case '"':
		case '\'':
			if (debug_mode < 2) {
				fprintf(ofd, "%-16s: %s", "Quoted string", curword);
				while ((i = gettok()) == TK_STR)
					outpos += fprintf(ofd, "%s", curword);
				if (i == '\n')
					fprintf(ofd, " --> EOL!!\n");
				else
					outpos += fprintf(ofd, "%s\n", curword);
				break;
			}
		/*FALLTHROUGH*/ default:
			fprintf(ofd, "%-16s: '", token_txn(i));
			{
				char *p;
				for (p = curword; *p; p++)
					if (isprint(*p) && *p != '\'' && *p != '\\')
						fputc(*p, ofd);
					else if (*p == '\n')
						fprintf(ofd, "\\n");
					else if (*p == '\t')
						fprintf(ofd, "\\t");
					else if (*p == '\v')
						fprintf(ofd, "\\v");
					else if (*p == '\b')
						fprintf(ofd, "\\b");
					else if (*p == '\r')
						fprintf(ofd, "\\r");
					else if (*p == '\f')
						fprintf(ofd, "\\f");
					else if (*p == '\a')
						fprintf(ofd, "\\a");
					else
						fprintf(ofd, "\\x%02x", (unsigned char) *p);
			}
			fprintf(ofd, "'\n");
			break;
		case TK_NUM:
			val = strtoul(curword, (void *) 0, 0);
			fprintf(ofd, "%-16s: ", token_txn(i));
			fprintf(ofd, "%s => %ld\n", curword, val);
			break;
		case TK_COPY:
			fprintf(ofd, "%-16s: ", token_txn(i));
			fprintf(ofd, "#%s\n", curword);
			break;
		case '\n':
			fprintf(ofd, "%-16s:\n", "Newline char");
			break;
		}
	}
}

char *token_txn(int token)
{
	char *s = "UNKNOWN";
	static char buf[17];

	if (token > ' ' && token <= '~') {
		sprintf(buf, "TK_CHAR('%c')", token);
		return buf;
	}
	if (token >= 0 && token < 0x100) {
		sprintf(buf, "TK_CHAR(%d)", token);
		return buf;
	}

	switch (token) {
	case TK_WSPACE:
		s = "TK_WSPACE";
		break;
	case TK_WORD:
		s = "TK_WORD";
		break;
	case TK_NUM:
		s = "TK_NUM";
		break;
	case TK_FLT:
		s = "TK_FLT";
		break;
	case TK_QUOT:
		s = "TK_QUOT";
		break;
	case TK_STR:
		s = "TK_STR";
		break;
	case TK_FILE:
		s = "TK_FILE";
		break;
	case TK_LINE:
		s = "TK_LINE";
		break;
	case TK_COPY:
		s = "TK_COPY";
		break;
	case TK_NE_OP:
		s = "TK_NE_OP";
		break;
	case TK_MOD_ASSIGN:
		s = "TK_MOD_ASSIGN";
		break;
	case TK_AND_OP:
		s = "TK_AND_OP";
		break;
	case TK_AND_ASSIGN:
		s = "TK_AND_ASSIGN";
		break;
	case TK_MUL_ASSIGN:
		s = "TK_MUL_ASSIGN";
		break;
	case TK_INC_OP:
		s = "TK_INC_OP";
		break;
	case TK_ADD_ASSIGN:
		s = "TK_ADD_ASSIGN";
		break;
	case TK_DEC_OP:
		s = "TK_DEC_OP";
		break;
	case TK_SUB_ASSIGN:
		s = "TK_SUB_ASSIGN";
		break;
	case TK_PTR_OP:
		s = "TK_PTR_OP";
		break;
	case TK_ELLIPSIS:
		s = "TK_ELLIPSIS";
		break;
	case TK_DIV_ASSIGN:
		s = "TK_DIV_ASSIGN";
		break;
	case TK_LEFT_OP:
		s = "TK_LEFT_OP";
		break;
	case TK_LEFT_ASSIGN:
		s = "TK_LEFT_ASSIGN";
		break;
	case TK_LE_OP:
		s = "TK_LE_OP";
		break;
	case TK_EQ_OP:
		s = "TK_EQ_OP";
		break;
	case TK_GE_OP:
		s = "TK_GE_OP";
		break;
	case TK_RIGHT_OP:
		s = "TK_RIGHT_OP";
		break;
	case TK_RIGHT_ASSIGN:
		s = "TK_RIGHT_ASSIGN";
		break;
	case TK_XOR_ASSIGN:
		s = "TK_XOR_ASSIGN";
		break;
	case TK_OR_ASSIGN:
		s = "TK_OR_ASSIGN";
		break;
	case TK_OR_OP:
		s = "TK_OR_OP";
		break;
	case TK_AUTO:
		s = "TK_AUTO";
		break;
	case TK_BREAK:
		s = "TK_BREAK";
		break;
	case TK_CASE:
		s = "TK_CASE";
		break;
	case TK_CHAR:
		s = "TK_CHAR";
		break;
	case TK_CONST:
		s = "TK_CONST";
		break;
	case TK_CONTINUE:
		s = "TK_CONTINUE";
		break;
	case TK_DEFAULT:
		s = "TK_DEFAULT";
		break;
	case TK_DO:
		s = "TK_DO";
		break;
	case TK_DOUBLE:
		s = "TK_DOUBLE";
		break;
	case TK_ELSE:
		s = "TK_ELSE";
		break;
	case TK_ENUM:
		s = "TK_ENUM";
		break;
	case TK_EXTERN:
		s = "TK_EXTERN";
		break;
	case TK_FLOAT:
		s = "TK_FLOAT";
		break;
	case TK_FOR:
		s = "TK_FOR";
		break;
	case TK_GOTO:
		s = "TK_GOTO";
		break;
	case TK_IF:
		s = "TK_IF";
		break;
	case TK_INT:
		s = "TK_INT";
		break;
	case TK_LONG:
		s = "TK_LONG";
		break;
	case TK_REGISTER:
		s = "TK_REGISTER";
		break;
	case TK_RETURN:
		s = "TK_RETURN";
		break;
	case TK_SHORT:
		s = "TK_SHORT";
		break;
	case TK_SIGNED:
		s = "TK_SIGNED";
		break;
	case TK_SIZEOF:
		s = "TK_SIZEOF";
		break;
	case TK_STATIC:
		s = "TK_STATIC";
		break;
	case TK_STRUCT:
		s = "TK_STRUCT";
		break;
	case TK_SWITCH:
		s = "TK_SWITCH";
		break;
	case TK_TYPEDEF:
		s = "TK_TYPEDEF";
		break;
	case TK_UNION:
		s = "TK_UNION";
		break;
	case TK_UNSIGNED:
		s = "TK_UNSIGNED";
		break;
	case TK_VOID:
		s = "TK_VOID";
		break;
	case TK_VOLATILE:
		s = "TK_VOLATILE";
		break;
	case TK_WHILE:
		s = "TK_WHILE";
		break;
	}
	return s;
}
#endif
