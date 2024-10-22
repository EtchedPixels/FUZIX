/* (C) Robert de Bath 1997 under the terms of the GPL.
 *
 * This is a manual pager program, it will search for and format manual
 * pages which it then pipes to more.
 *
 * The program understands manual pages that have been compressed with
 * either 'compress' or 'gzip' and will decompress them on the fly.
 *
 * The environment is checked for these variables:
 *   MANSECT=1:2:3:4:5:6:7:8:9		# Manual section search order.
 *   MANPATH=/usr/local/man:/usr/man	# Directorys to search for man tree.
 *   PAGER=more				# pager progam to use.
 *   PATH=...				# Search for gzip/uncompress
 *
 * The program will display documents that are either in it's own "nroff -man"
 * like format or in "catman" format, it will not correctly display pages in
 * the BSD '-mdoc' format.
 *
 * Neither nroff nor any similar program is needed as this program has it's
 * own built in _man_ _page_ _formatter_. This is NOT an nroff clone and will
 * not (for instance) understand macros or tbl constructs.
 *
 * Unlike groff this is small, fast and picky!
 */

#undef SPLATTER			/* Define for lots of messages out - for debug */

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <unistd.h>
#endif
#include <ctype.h>
#include <string.h>

FILE *ofd;
FILE *ifd;
int ifd_class = 0;		/* Type of ifd, 0=stdin, 1=file, 2=pipe */

char whitespace[256];
char word[80];			/* Current word */
int no_nl = 1;			/* Next NL in input file is ignored */
int catmode = 1;		/* Have we seen a '.XX' command ? */

int keep_nl = 0;		/* How many nl to keep til eof */
int optional_keep = 0;		/* Is the next keep optional ? */
int pending_nl = 0;		/* Is there a pending newline on output? */
int no_fill = 0;		/* Disable 'filling' of lines */
int right_adjust = 1;		/* Adjust right margin */

int standard_tab = 5;		/* Amount left margin stepped by */
int left_indent = 0;		/* Current step of left margin */
int old_para_indent = 0;	/* Indent to go back to after this paragraph */
int next_line_indent = -1;	/* Indent after next line_break */
int page_no = 1;		/* Page number */
int input_tab = 8;		/* Tab width in input file */

int right_margin = 65;		/* Don't print past this column */
int page_length = 66;		/* Lines on page */
int current_line = 0;		/* Line number = ? */
int gaps_on_line = 0;		/* Gaps on line for adjustments */

int *line_ptr = 0;
int line[256];			/* Buffer for building output line */
int cur_font = 0x100;		/* Current font, 1 == Roman */

char line_header[256] = "";	/* Page header line */
char line_footer[256] = "";	/* Page footer line */
char little_header[256] = "";	/* Mini header for tty mode */

char man_file[256] = "";

int flg_w = 0;
int verbose = 1;
int local = 0;


/* Prototypes */
void do_file(void);
void do_skipeol(void);
int do_fontwords(int this_font, int other_font, int early_exit);
void print_word(char *pword);
void line_break(void);
void page_break(void);
void print_header(void);
void print_footer(void);
void step(char **pcurr, char **pnext);
int open_page(char *name);
void close_page(void);
int fetch_word(void);
int do_command(void);
int do_argvcmd(int cmd_id);
int do_noargs(int cmd_id);
void build_headers(void);

int find_page(char *name, char *sect)
{
	static char defpath[] = "/usr/local/man:/usr/man";
	static char defsect[] = "1:2:3:4:5:6:7:8:9";
	static char defsuff[] = ":.gz:.Z";
	static char manorcat[] = "man:cat";

	static char fbuf[256];
	char *manpath;
	char *mansect = sect;
	char *mansuff;
	char *mc, *mp, *ms, *su, *nmc, *nmp, *nms, *nsu;
	int rv = -1;

	if (local) {
		if ((rv = open_page(name)) < 0) {
			fprintf(stderr, "File not found: %s\n", name);
			exit(1);
		}
		if (flg_w) {
			printf("%s\n", name);
			close(rv);
			return 0;
		}
		return rv;
	}

	manpath = getenv("MANPATH");
	if (!manpath)
		manpath = defpath;
	if (!mansect)
		mansect = getenv("MANSECT");
	if (!mansect)
		mansect = defsect;
	mansuff = defsuff;

	if (strchr(name, '/')) {
		for (su = nsu = mansuff, step(&su, &nsu); su;
		     step(&su, &nsu)) {
			strcpy(fbuf, name);
			strcat(fbuf, su);
			if ((rv = open_page(fbuf)) >= 0)
				break;
		}
		*man_file = 0;
		return rv;
	}

	/* SEARCH!! */
	for (mc = nmc = manorcat, step(&mc, &nmc); mc; step(&mc, &nmc))
		for (ms = nms = mansect, step(&ms, &nms); ms;
		     step(&ms, &nms))
			for (mp = nmp = manpath, step(&mp, &nmp); mp;
			     step(&mp, &nmp))
				for (su = nsu = mansuff, step(&su, &nsu);
				     su; step(&su, &nsu)) {
					sprintf(fbuf, "%s/%s%s/%s.%s%s",
						mp, mc, ms, name, ms, su);

					/* Got it ? */
					if (access(fbuf, 0) < 0)
						continue;
					if (flg_w) {
						printf("%s\n", fbuf);
						rv = 0;
						continue;
					}

					/* Try it ! */
					if ((rv = open_page(fbuf)) >= 0) {
						char *p;
						strcpy(man_file, fbuf);
						p = strrchr(man_file, '/');
						if (p)
							*p = 0;
						p = strrchr(man_file, '/');
						if (p)
							p[1] = 0;
						return 0;
					}
				}

	return rv;
}

void step(char **pcurr, char **pnext)
{
	char *curr = *pcurr;
	char *next = *pnext;

	if (curr == 0)
		return;
	if (curr == next) {
		next = strchr(curr, ':');
		if (next)
			*next++ = 0;
	} else {
		curr = next;
		if (curr) {
			curr[-1] = ':';
			next = strchr(curr, ':');
			if (next)
				*next++ = 0;
		}
	}

	*pcurr = curr;
	*pnext = next;
}

int open_page(char *name)
{
	char *p;
	const char *command = 0;
	static char buf[256];

	if (access(name, 0) < 0)
		return -1;

	p = strrchr(name, '.');
	if (p) {
		if (strcmp(p, ".gz") == 0)
			command = "gzip -dc ";
		if (strcmp(p, ".Z") == 0)
			command = "uncompress -c ";
	}

	if (command) {
		strcpy(buf, command);
		strcat(buf, name);
		ifd = popen(buf, "r");
		if (ifd == 0)
			return -1;
		ifd_class = 2;
		return 0;
	}
	ifd = fopen(name, "r");
	if (ifd == 0)
		return -1;
	ifd_class = 1;
	return 0;
}

void close_page(void)
{
	switch (ifd_class) {
	case 1:
		fclose(ifd);
		break;
	case 2:
		pclose(ifd);
		break;
	}
	ifd_class = 0;
	return;
}

/****************************************************************************
 * Accepted nroff commands and executors.
 */

struct cmd_list_s {
	char cmd[3];
	int class;
	int id;
} cmd_list[] = {
	{
	"\\\"", 0, 0}, {
	"nh", 0, 0},		/* This program never inserts hyphens */
	{
	"hy", 0, 0},		/* This program never inserts hyphens */
	{
	"PD", 0, 0},		/* Inter-para distance is 1 line */
	{
	"DT", 0, 0},		/* Default tabs, they can't be non-default! */
	{
	"IX", 0, 0},		/* Indexing for some weird package */
	{
	"Id", 0, 0},		/* Line for RCS tokens */
	{
	"BY", 0, 0},		/* I wonder where this should go ? */
	{
	"nf", 0, 1},		/* Line break, Turn line fill off */
	{
	"fi", 0, 2},		/* Line break, Turn line fill on */
	{
	"sp", 0, 3},		/* Line break, line space (arg for Nr lines) */
	{
	"br", 0, 4},		/* Line break */
	{
	"bp", 0, 5},		/* Page break */
	{
	"PP", 0, 6}, {
	"LP", 0, 6}, {
	"P", 0, 6},		/* Paragraph */
	{
	"RS", 0, 7},		/* New Para + Indent start */
	{
	"RE", 0, 8},		/* New Para + Indent end */
	{
	"HP", 0, 9},		/* Begin hanging indent (TP without arg?) */
	{
	"ad", 0, 10},		/* Line up right margin */
	{
	"na", 0, 11},		/* Leave right margin unaligned */
	{
	"ta", 0, 12},		/* Changes _input_ tab spacing, right? */
	{
	"TH", 1, 1},		/* Title and headers */
	{
	"SH", 1, 2},		/* Section */
	{
	"SS", 1, 3},		/* Subsection */
	{
	"IP", 1, 4},		/* New para, indent except argument 1 */
	{
	"TP", 1, 5},		/* New para, indent except line 1 */
	{
	"B", 2, 22},		/* Various font fiddles */
	{
	"BI", 2, 23}, {
	"BR", 2, 21}, {
	"I", 2, 33}, {
	"IB", 2, 32}, {
	"IR", 2, 31}, {
	"RB", 2, 12}, {
	"RI", 2, 13}, {
	"SB", 2, 42}, {
	"SM", 2, 44}, {
	"C", 2, 22},		/* PH-UX manual pages! */
	{
	"CI", 2, 23}, {
	"CR", 2, 21}, {
	"IC", 2, 32}, {
	"RC", 2, 12}, {
	"so", 3, 0}, {
	"\0\0", 0}
};

/****************************************************************************
 * ifd is the manual page, ofd is the 'output' file or pipe, format it!
 */
void do_file(void)
{
	int nl;
	ungetc('\r', ifd);

	while ((nl = fetch_word()) >= 0) {
#ifdef SPLATTER
		fprintf(ofd, ">WS='%s',", whitespace);
		fprintf(ofd, "catmode=%d,", catmode);
		fprintf(ofd, "nl=%d,", nl);
		fprintf(ofd, "no_nl=%d,", no_nl);
		fprintf(ofd, "no_fill=%d,", no_fill);
		fprintf(ofd, "keep_nl=%d,", keep_nl);
		fprintf(ofd, "opt_keep=%d,", optional_keep);
		fprintf(ofd, "WD='%s',", word);
		fprintf(ofd, "\n");
#endif

		if (catmode) {
			if (strcmp(word, "'\\\"") == 0
			    || strcmp(word, "'''") == 0) {
				/* This is a marker sometimes used for opening subprocesses like
				 * tbl and equ; this program ignores it.
				 */
				do_skipeol();
			} else if (*whitespace == '\r')
				fprintf(ofd, "%s%s", whitespace + 1, word);
			else
				fprintf(ofd, "%s%s", whitespace, word);
		} else {
			if (keep_nl && nl && !no_nl) {
				if (optional_keep) {
					optional_keep = 0;
					if (line_ptr == 0
					    || next_line_indent < 0
					    || left_indent + (line_ptr -
							      line) + 1 >
					    next_line_indent)
						line_break();
					else if (line_ptr != 0
						 && next_line_indent > 0) {
						while (left_indent +
						       (line_ptr - line) +
						       1 <=
						       next_line_indent)
							*line_ptr++ =
							    cur_font + ' ';
					}
				} else
					line_break();
				if (keep_nl > 0)
					keep_nl--;
			}

			if (nl == 1 && (word[0] == '.' ||
					(word[0] == '\''
					 && strcmp(word, "'\\\"") == 0)
					|| (word[0] == '\''
					    && strcmp(word, "'''") == 0)
			    )) {
				no_nl = 1;
				if (do_command() < 0)
					break;
			} else {
				if (nl == 1 && no_fill)
					line_break();
				if (*whitespace)
					print_word(whitespace);
				print_word(word);
				no_nl = 0;
			}
		}
	}

	page_break();
}

int fetch_word(void)
{
	static int col = 0;
	char *p;
	int ch, nl;

	nl = 0;
	*(p = whitespace) = 0;

	if (!catmode && !no_fill)
		p++;

	while ((ch = fgetc(ifd)) != EOF && isspace(ch)) {
		if (nl && no_fill && ch != '.' && ch != '\n')
			break;
		if (nl && !catmode && ch == '\n') {
			*whitespace = 0;
			strcpy(word, ".sp");
			ungetc(ch, ifd);
			return 1;
		}
		nl = (ch == '\n' || ch == '\r');
		if (nl)
			col = 0;
		else
			col++;

		if (no_fill && nl && *whitespace) {
			*word = 0;
			ungetc(ch, ifd);
			return 0;
		}

		if (p < whitespace + sizeof(whitespace) - 1
		    && (!nl || catmode))
			*p++ = ch;

		if (ch == '\t' && !catmode) {
			p[-1] = ' ';
			while (col % input_tab) {
				if (p <
				    whitespace + sizeof(whitespace) - 1)
					*p++ = ' ';
				col++;
			}
		}

		if (!catmode && !no_fill && nl)
			*(p = whitespace) = 0;
	}
	*p = 0;

	if (catmode && ch == '.' && nl)
		catmode = 0;

	*(p = word) = 0;
	if (ch == EOF || p > word + sizeof(word) / 2) {
		if (p != word) {
			ungetc(ch, ifd);
			*p = 0;
			return nl;
		}
		return -1;
	}
	ungetc(ch, ifd);

	while ((ch = fgetc(ifd)) != EOF && !isspace(ch)) {
		if (p < word + sizeof(word) - 1)
			*p++ = ch;
		col++;
		if (ch == '\\') {
			if ((ch = fgetc(ifd)) == EOF)
				break;
			/* if (ch == ' ') ch = ' ' + 0x80;   */  /* XXX Is this line needed? */
			if (p < word + sizeof(word) - 1)
				*p++ = ch;
			col++;
		}
	}
	*p = 0;
	ungetc(ch, ifd);

	return (nl != 0);
}

int do_command(void)
{
	const char *cmd;
	int i;
	char lbuf[10];

	cmd = word + 1;

	/* Comments don't need the space */
	if (strncmp(cmd, "\\\"", 2) == 0)
		cmd = "\\\"";

	for (i = 0; cmd_list[i].cmd[0]; i++) {
		if (strcmp(cmd_list[i].cmd, cmd) == 0)
			break;
	}

	if (cmd_list[i].cmd[0] == 0) {
		if (verbose) {
			strncpy(lbuf, cmd, 3);
			lbuf[3] = 0;
			line_break();
			i = left_indent;
			left_indent = 0;
			strcpy(word, "**** Unknown formatter command: .");
			strcat(word, lbuf);
			print_word(word);
			line_break();
			left_indent = i;
		}

		i = 0;		/* Treat as comment */
	}

	switch (cmd_list[i].class) {
	case 1:		/* Parametered commands */
		return do_argvcmd(cmd_list[i].id);

	case 2:		/* Font changers */
		return do_fontwords(cmd_list[i].id / 10,
				    cmd_list[i].id % 10, 0);

	case 3:		/* .so */
		fetch_word();
		strcat(man_file, word);
		close_page();
		if (find_page(man_file, (char *) 0) < 0) {
			fprintf(stderr, "Cannot open .so file %s\n", word);
			return -1;
		}
		ungetc('\r', ifd);
		break;

	default:
		do_skipeol();
		if (cmd_list[i].id)
			return do_noargs(cmd_list[i].id);
	}
	return 0;
}

void do_skipeol(void)
{
	int ch;
	char *p = word;

	while ((ch = fgetc(ifd)) != EOF && ch != '\n')
		if (p < word + sizeof(word) - 1)
			*p++ = ch;
	*p = 0;
	ungetc(ch, ifd);
	return;
}

int do_fontwords(int this_font, int other_font, int early_exit)
{
	static char ftab[] = " RBIS";
	char *p = word;
	int i, ch;
	int in_quote = 0;

	no_nl = 0;		/* Line is effectivly been reprocessed so NL is visable */
	for (;;) {
		if (p == word) {
			strcpy(p, "\\f");
			p[2] = ftab[this_font];
			p += 3;
		}
		if ((ch = fgetc(ifd)) == EOF || ch == '\n')
			break;
		if (ch == '"') {
			in_quote = !in_quote;
			continue;
		}
		if (in_quote || !isspace(ch)) {
			if (isspace(ch) && p > word + 3) {
				strcpy(p, "\\fR");
				p += 3;
				*p = 0;
				print_word(word);
				p = word;
				if (no_fill)
					print_word(" ");
				continue;
			}
			if (p < word + sizeof(word) - 4)
				*p++ = ch;
			if (ch == '\\') {
				if ((ch = fgetc(ifd)) == EOF || ch == '\n')
					break;
				if (p < word + sizeof(word) - 4)
					*p++ = ch;
			}
			continue;
		}

		if (p != word + 3) {
			if (early_exit)
				break;

			if (this_font == other_font) {
				strcpy(p, "\\fR");
				p += 3;
				*p = 0;
				print_word(word);
				p = word;
			}
			i = this_font;
			this_font = other_font;
			other_font = i;
			if (p < word + sizeof(word) - 4) {
				strcpy(p, "\\f");
				p[2] = ftab[this_font];
				p += 3;
			}
		}
	}
	ungetc(ch, ifd);

	if (p > word + 3) {
		strcpy(p, "\\fR");
		p += 3;
		*p = 0;
		print_word(word);
	}

	return 0;
}

int do_noargs(int cmd_id)
{
	if (cmd_id < 10)
		line_break();
	switch (cmd_id) {
	case 1:
		no_fill = 1;
		break;
	case 2:
		no_fill = 0;
		break;
	case 3:
		pending_nl = 1;
		break;
	case 4:
		break;
	case 5:
		page_break();
		break;
	case 6:
		left_indent = old_para_indent;
		pending_nl = 1;
		break;
	case 7:
		pending_nl = 1;
		left_indent += standard_tab;
		old_para_indent += standard_tab;
		break;
	case 8:
		pending_nl = 1;
		left_indent -= standard_tab;
		old_para_indent -= standard_tab;
		break;

	case 10:
		right_adjust = 1;
		break;
	case 11:
		right_adjust = 0;
		break;
	case 12:
		input_tab = atoi(word);
		if (input_tab <= 0)
			input_tab = 8;
		break;
	}
	return 0;
}

int do_argvcmd(int cmd_id)
{
	int ch;

	line_break();
	while ((ch = fgetc(ifd)) != EOF && (ch == ' ' || ch == '\t'));
	ungetc(ch, ifd);

	switch (cmd_id + 10 * (ch == '\n')) {
	case 1:		/* Title and headers */
		page_break();
		left_indent = old_para_indent = standard_tab;
		build_headers();
		break;

	case 2:		/* Section */
		left_indent = 0;
		next_line_indent = old_para_indent = standard_tab;
		no_nl = 0;
		keep_nl = 1;
		pending_nl = 1;

		do_fontwords(1, 1, 0);
		return 0;
	case 3:		/* Subsection */
		left_indent = standard_tab / 2;
		next_line_indent = old_para_indent = standard_tab;
		no_nl = 0;
		keep_nl = 1;
		pending_nl = 1;

		do_fontwords(1, 1, 0);
		break;

	case 15:
	case 5:		/* New para, indent except line 1 */
		do_skipeol();
		next_line_indent = old_para_indent + standard_tab;
		left_indent = old_para_indent;
		pending_nl = 1;
		keep_nl = 1;
		optional_keep = 1;
		break;

	case 4:		/* New para, indent except argument 1 */
		next_line_indent = old_para_indent + standard_tab;
		left_indent = old_para_indent;
		pending_nl = 1;
		keep_nl = 1;
		optional_keep = 1;
		do_fontwords(1, 1, 1);
		do_skipeol();
		break;

	case 14:
		pending_nl = 1;
		left_indent = old_para_indent + standard_tab;
		break;
	}

	return 0;
}

void build_headers(void)
{
	static char buffer[5][80];
	int strno = 0, stroff = 0;
	int last_ch = 0, ch, in_quote = 0;

	for (ch = 0; ch < 5; ch++)
		buffer[ch][0] = 0;

	for (;;) {
		if ((ch = fgetc(ifd)) == EOF || ch == '\n')
			break;
		if (ch == '"') {
			if (last_ch == '\\') {
				stroff--;
				break;
			}
			in_quote = !in_quote;
			continue;
		}
		last_ch = ch;
		if (in_quote || !isspace(ch)) {
			/* Nb, this does nothing about backslashes, perhaps it should */
			if (stroff < sizeof(buffer[strno]) - 1)
				buffer[strno][stroff++] = ch;
			continue;
		}
		buffer[strno][stroff] = 0;

		if (buffer[strno][0]) {
			strno++;
			stroff = 0;
			if (strno == 5)
				break;
		}
	}
	if (strno < 5)
		buffer[strno][stroff] = 0;
	ungetc(ch, ifd);

	/* Ok we should have upto 5 arguments build the header and footer */

	memset(little_header, ' ', right_margin);

	memset(line_header, ' ', right_margin);
	strcpy(line_header, buffer[0]);
	strcat(line_header, "(");
	strcat(line_header, buffer[1]);
	strcat(line_header, ")");
	ch = strlen(line_header);
	strcpy(line_header + right_margin - ch, line_header);
	line_header[ch] = ' ';

	ch = strlen(buffer[4]);
	if (ch > right_margin)
		ch = right_margin - 12;
	memcpy(line_header + right_margin / 2 - ch / 2, buffer[4], ch);

	memcpy(little_header, line_header, right_margin / 2 + ch / 2 + 1);
	strcpy(little_header + right_margin - strlen(buffer[2]),
	       buffer[2]);

	memset(line_footer, ' ', right_margin - 6);
	line_footer[right_margin - 6] = 0;
	memcpy(line_footer, buffer[3], strlen(buffer[3]));

	ch = strlen(buffer[2]);
	if (ch > right_margin)
		ch = right_margin - 12;
	memcpy(line_footer + right_margin / 2 - ch / 2, buffer[2], ch);

	do_skipeol();
}

void print_word(char *pword)
{
/* Eat   \&  \a .. \z and \A .. \Z
 * \fX   Switch to font X (R B I S etc)
 * \(XX  Special character XX
 * \X    Print as X
 */

	char *s;
	int *d, ch = 0;
	int length = 0;
	static int wword[256];
	int sp_font = cur_font;

	/* Eat and translate characters. */
	for (s = pword, d = wword; *s; s++) {
		ch = 0;
		if (*s == '\n')
			continue;
		if (*s != '\\') {
			*d++ = (ch = *s) + cur_font;
			length++;
		} else {
			if (s[1] == 0)
				break;
			s++;
			if (*s == 'f') {
				if (s[1]) {
					static char fnt[] = " RBI";
					char *p = strchr(fnt, *++s);
					if (p == 0)
						cur_font = 0x100;
					else
						cur_font =
						    0x100 * (p - fnt);
				}
				continue;
			} else if (*s == 's') {
				/* Font size adjusts - strip */
				while (s[1]
				       && strchr("+-0123456789", s[1]))
					s++;
				continue;
			} else if (isalpha(*s) || strchr("!&^[]|~", *s))
				continue;
			else if (*s == '(' || *s == '*') {
				/* XXX Humm character xlate */

				if (*s == '*')
					if (s[1])
						++s;
				if (s[1])
					++s;
				if (s[1])
					++s;
				*d++ = '*' + cur_font;
				length++;
				continue;
			}

			*d++ = *s + cur_font;
			length++;
		}
	}

	*d = 0;
#ifdef SPLATTER
	{
		int *x;
		fprintf(ofd, ">WORD:");
		for (x = wword; *x; x++)
			fputc(*x, ofd);
		fprintf(ofd, ":\n");
	}
#endif

	if (*wword == 0)
		return;

	if (line_ptr)
		if (line_ptr + ((line_ptr[-1] & 0xFF) == '.') - line +
		    length >= right_margin - left_indent) {
			right_adjust = -right_adjust;
			line_break();
		}

	if (line_ptr == 0)
		line_ptr = line;
	else if (!no_fill && (line_ptr[-1] & 0xFF) > ' ') {
		if ((line_ptr[-1] & 0xFF) == '.')
			*line_ptr++ = cur_font + ' ';
		*line_ptr++ = sp_font;
		gaps_on_line++;
	}

	memcpy(line_ptr, wword, length * sizeof(int));
	line_ptr += length;
}

void line_break(void)
{
	int *d, ch;
	int spg = 1, rspg = 1, spgs = 0, gap = 0;

	if (line_ptr == 0)
		return;

	if (current_line == 0)
		print_header();

	if (page_length > 12 &&
	    current_line + pending_nl > page_length - 6) {
		print_footer();
		print_header();
	}

	if (current_line)
		current_line += 1 + pending_nl;
	for (; pending_nl > 0; pending_nl--)
		fprintf(ofd, "\n");

	if (right_adjust < 0) {
		int over = right_margin - left_indent - (line_ptr - line);
#ifdef SPLATTER
		fprintf(ofd, ">Gaps=%d, Over=%d, ", gaps_on_line, over);
#endif
		if (gaps_on_line && over) {
			spg = rspg = 1 + over / gaps_on_line;
			over = over % gaps_on_line;
			if (over) {
				if (current_line % 2) {
					spgs = over;
					spg++;
				} else {
					spgs = gaps_on_line - over;
					rspg++;
				}
			}
		}
#ifdef SPLATTER
		fprintf(ofd, " (%d,%d) sw=%d\n", spg, rspg, spgs);
#endif
		right_adjust = 1;
	}

	*line_ptr = 0;
	if (*line)
		for (ch = left_indent; ch > 0; ch--)
			fputc(' ', ofd);

	for (d = line; *d; d++) {
		ch = *d;
		if ((ch & 0xFF) == 0) {
			int i;
			if (gap++ < spgs)
				i = spg;
			else
				i = rspg;
			for (; i > 0; i--)
				fputc(' ', ofd);
		} else
			switch (ch >> 8) {
			case 2:
				fputc(ch & 0xFF, ofd);
				fputc('\b', ofd);
				fputc(ch & 0xFF, ofd);
				break;
			case 3:
				fputc('_', ofd);
				fputc('\b', ofd);
				fputc(ch & 0xFF, ofd);
				break;
			default:
				fputc(ch & 0xFF, ofd);
				break;
			}
	}
	fputc('\n', ofd);

	line_ptr = 0;

	if (next_line_indent > 0)
		left_indent = next_line_indent;
	next_line_indent = -1;
	gaps_on_line = 0;
}

void page_break(void)
{
	line_break();
	if (current_line)
		print_footer();
}

void print_header(void)
{
	pending_nl = 0;

	if (*line_header && page_length) {
		current_line = 7;
		fprintf(ofd, "\n\n\n%s\n\n\n", line_header);
	} else if (*little_header && !page_length) {
		current_line = 1;
		fprintf(ofd, "%s\n\n", little_header);
	}
}

void print_footer(void)
{
	if (!page_length)
		return;

	while (current_line <= page_length - 3) {
		fputc('\n', ofd);
		current_line++;
	}

	fprintf(ofd, "%s%6d\n\n\n", line_footer, page_no++);
	current_line = 0;
}

/****************************************************************************
 * Main routine, hunt down the manpage.
 */
int main(int argc, char **argv)
{
	int do_pclose_ofd = 0;
	int ar;
	char *mansect = 0;
	char *manname = 0;
	ofd = stdout;
	ifd = stdin;

	for (ar = 1; ar < argc; ar++)
		if (argv[ar][0] == '-') {
			char *p;
			for (p = argv[ar] + 1; *p; p++)
				switch (*p) {
				case 'w':
					flg_w = 1;
					break;
				case 'v':
					verbose = 1;
					break;
				case 'q':
					verbose = 0;
					break;
				case 'l':
					local = 1;
					break;
				}
		} else if (isdigit(argv[ar][0]))
			mansect = argv[ar];
		else if (manname == 0)
			manname = argv[ar];
		else if (mansect == 0) {
			mansect = manname;
			manname = argv[ar];
		} else {
			fprintf(stderr, "Ignoring argument %s\n",
				argv[ar]);
			break;
		}

	if (manname == 0) {
		fprintf(stderr, "Which manpage ?\n");
		exit(1);
	}

	if (find_page(manname, mansect) < 0) {
		if (mansect != NULL)
			fprintf(stderr,
				"No entry for %s in section %s of the manual.\n",
				manname, mansect);
		else
			fprintf(stderr, "No manual entry for %s\n",
				manname);
		exit(1);
	}
	if (flg_w)
		exit(0);

	/* ifd is now the file - display it */
	if (isatty(1)) {	/* If writing to a tty try to do it to a pager */
		/* The original way that this was coded relied on popen returning
		   NULL if PAGER was undefined. However, both on Linux and on
		   the V7 sh in FUZIX, sh barfed when getenv returned NULL:
		   sh: 0: -c requires an argument
		 */
		if (getenv("PAGER")) {
			ofd = popen(getenv("PAGER"), "w");
		} else {
			ofd = popen("more", "w");
		}
		if (ofd == 0) {
			/* This is likely unreachable because the popen will croak
			   in the absence of more:
			   sh: 1: tmore: not found
			   so we don't get the chance to fall-back to stdout
			 */
			ofd = stdout;
		} else {
			do_pclose_ofd = 1;
			page_length = 0;
			right_margin = 78;
		}
	}

	do_file();

	/* Close files */
	if (do_pclose_ofd)
		pclose(ofd);
	close_page();
	exit(0);
}
