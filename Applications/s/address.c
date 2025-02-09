/*
* address.c - process addresses
*
*
* Entry point:
*
*	address(n, c, op)
*	int n;
*	char c, op;
*		Reposition the cursor as specified by the count n, the motion
*		command c and the operator op.  For pure cursor movement,
*		op = ' ' (the blank character).  For line addresses used with an
*		operator (op != ' '), the cursor position in the addressed line
*		is set to -1.  If the addressing operation fails, then the
*		cursor's location is unchanged.
*
*	A list of the addressing commands follows.  The default value of the
*	count <n> is 1, except for commands g (where it is the number of lines
*	in the buffer) and ctrl('d') and ctrl('u') (where it is half of a screen).
*	Commands M, 0 (zero), ' (apostrophe), and ` (backquote) ignore the count.
*
*			Line Addresses:
*	<n>g		- line n of the buffer
*	<n>H		- down n lines from the top of the screen
*	<n>L		- up n lines from the bottom of the screen
*	M		- the middle line of the screen
*	<n><return>	- down n lines from the current line
*	<n>-		- up n lines from the current line
*	<n>ctrl(d)	- down n lines from the bottom of the screen
*	<n>ctrl(u)	- up n lines from the top of the screen
*	<n>c		- down n-1 lines (only with the c operator)
*	<n>d		- down n-1 lines (only with the d operator)
*	<n>y		- down n-1 lines (only with the y operator)
*
*			Character Addresses:
*	<n>b		- back n words
*	<n>f<char>	- right n occurrences of <char>
*	<n>F<char>	- left n occurrences of <char>
*	<n>h		- left n characters (same as <n><backspace>)
*	<n>j		- down n lines in the same column
*	<n>k		- up n lines in the same column
*	<n>l		- right n characters (same as <n><space>)
*	<n>n		- n repetetions of the previous pattern search
*	<n>w		- forward n words
*	0 (zero)	- the start of the current line
*	<n>$		- the end of the (n-1)th following line
*	<n><space>	- right n characters
*	<n><backspace>	- left n characters
*	'  (apostrophe)	- return to the marked location
*	`  (backquote)	- return to the previous location
*	<n>;		- n repetetions of the previous f or F command
*	<n>/<string><return>	- forward n occurrences of string
*	<n>\<string><return>	- back n occurrences of string
*
*
* External procedure calls:
*
*	b_getcur(line_ptr, pos_ptr)			.. file Bman.c
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the cursor.
*
*	b_getmark(line_ptr, pos_ptr)			.. file Bman.c
*	int *line_ptr, *pos_ptr;
*		Return the line and position of the mark.
*
*	b_gets(k, s)					.. file Bman.c
*	int k;
*	char s[];
*		Copy the k-th buffer line to s.
*
*	b_setcur(line, pos)				.. file Bman.c
*	int line, pos;
*		Set the cursor location.
*
*	b_setline(line)					.. file Bman.c
*	int line;
*		Set the cursor to line's first nonwhite character.
*
*	int b_size()					.. file Bman.c
*		Return the number of lines in the buffer.
*
*	int k_getch()					.. file keyboard.c
*	   	Return the next character from the keyboard.
*
*	char k_lastcmd()				.. file keyboard.c
*	   	Return the first letter in the last command.
*
*	int s_firstline()				.. file Sman.c
*		Return the number of the first line visible on the screen.
*
*	char *s_getmsg(msg)				.. file Sman.c
*	char *msg;
*		Print msg; return the user's reply.
*
*	int s_lastline()				.. file Sman.c
*		Return the number of the last line visible on the screen.
*/

#include <string.h>

#include "s.h"

/* define "identifier character" and "special character" */
#define ident_char(x) (isalnum(x) || x == '_')
#define special_char(x) (!ident_char(x) && !isspace(x))

extern void b_getcur(), b_setcur(), b_gets(), b_getmark(), b_setline();
extern char k_lastcmd(), *s_getmsg();
extern int b_size(), s_firstline(), s_lastline(), k_getch();
static void do_up_down(), loc_char(), loc_word();
static void loc_string();
static int col_to_pos(), pos_to_col(), word_start(), locate();

void address(n, c, op)
int n;
char c, op;
{
	static int prev_line = 0, prev_pos, scroll_size = SCROLL_SIZE;
	int cur_line, cur_pos, direction, limit, line_addr, mark_line, mark_pos,
		new_line, new_pos;
	char ch, text[MAXTEXT-1];

	/* set default count to 1, except for these special cases */
	if (n == 0 && c != 'g' && c != ctrl('d') && c != ctrl('u')
			       && c != ctrl('f') && c != ctrl('b'))
		n = 1;
	b_getcur(&cur_line, &cur_pos);	/* cursor location */
	line_addr = 0;	/* reset by commands that address lines */

	switch (c) {

	/* ----------  Line Addresses:  ---------- */
		case 'g':
			/* ad hoc default value for the count */
			if (n == 0)
				n = b_size();
			line_addr = n;
			break;
		case 'H':
			line_addr = s_firstline() + n - 1;
			break;
		case 'L':
			line_addr = max (s_lastline() - n + 1, 1);
			break;
		case 'M':
			line_addr = (s_firstline() + s_lastline())/2;
			break;
		case CR:	/* <return> */
			line_addr = cur_line + n;
			break;
		case '-':
			line_addr = max (cur_line - n, 1);
			break;
		case ctrl('d'):
			/* ad hoc interpretation of the count */
			if (n != 0)
				scroll_size = n;
			line_addr = s_lastline() + scroll_size;
			break;
		case ctrl('u'):
			if (n != 0)
				scroll_size = n;
			line_addr = max (s_firstline() - scroll_size, 1);
			break;
		case ctrl('f'):
			/* ad hoc interpretation of the count */
			scroll_size = 24;
			if (n != 0)
				scroll_size = n;
			line_addr = s_lastline() + scroll_size;
			break;
		case ctrl('b'):
			scroll_size = 24;
			if (n != 0)
				scroll_size = n;
			line_addr = max (s_firstline() - scroll_size, 1);
			break;
  		case 'c':
  		case 'd':
  		case 'y':
  			if (op == c)
  				line_addr = cur_line + n - 1;
  			break;

	/* ----------  Character Addresses:  ---------- */
		case 'b':
			while (n-- > 0)
				loc_word(-1);
			/*
			*		ad hoc rule:
			* operators affect only the current line
			*/
			if (op != ' ') {
				b_getcur(&new_line, &new_pos);
				if (new_line != cur_line)
					b_setcur(cur_line, 0);
			}
			break;
		case 'f':
		case 'F':
			direction = (c == 'f') ? 1 : -1;
			ch = k_getch();
			while (n-- > 0)
				loc_char(ch, direction);
			break;
		case 'h':
		case '\b':	/* <backspace> */
			b_setcur(cur_line, max(cur_pos - n, 0));
			break;
		case 'j':
			do_up_down(n);
			break;
		case 'k':
			do_up_down(-n);
			break;
		case 'l':
		case ' ':
			b_gets(cur_line, text);
			limit = strlen(text) - 1;
			/*
			*		ad hoc rule:
			* operators affect the line's last character
			*/
			if (op != ' ')
				++limit;

			b_setcur(cur_line, min(cur_pos + n, limit));
			break;
		case 'n':
			while (n-- > 0)
				loc_string('\0');
			break;
		case 'w':
			while (n-- > 0)
				loc_word(1);
			/*
			*		ad hoc rule:
			* operators affect only the current line
			*/
			if (op != ' ') {
				b_getcur(&new_line, &new_pos);
				if (new_line != cur_line ||
				new_pos == cur_pos) {  /* last word in buffer */
					/* set cursor past the end of line */
					b_gets(cur_line, text);
					b_setcur(cur_line, strlen(text));
				}
			}
			/*
			*		ad hoc rule:
			* c does not affect the whitespace at the end of a word
			*/
			if (op == 'c') {
				b_getcur(&new_line, &new_pos);
				b_gets(new_line, text);
				while (new_pos > 0 && isspace(text[new_pos-1]))
					--new_pos;
				b_setcur(new_line, new_pos);
			}
			break;
		case '0':	/* zero */
			b_setcur(cur_line, 0);
			break;
		case '$':
			new_line = cur_line + n - 1;
			b_gets(new_line, text);
			new_pos = strlen(text) - 1;
			/*
			*		ad hoc rule:
			* operators affect the line's last character
			*/
			if (op != ' ')
				++new_pos;

			b_setcur(new_line, new_pos);
			break;
		case '\'':	/* apostrophe */
			b_getmark(&mark_line, &mark_pos);
			if (mark_line == 0)
				break;
			b_setcur(mark_line, mark_pos);
			/*
			*		ad hoc rule:
			* operators treat the marked location as a line address
			*/
			if (op != ' ')
				line_addr = mark_line;

			break;
		case '`':	/* backquote */
			if (prev_line == 0)
				break;
			b_setcur(prev_line, prev_pos);
			/*
			*		ad hoc rule:
			* operators treat the previous location as a line address
			*/
			if (op != ' ')
				line_addr = prev_line;

			break;
		case ';':
			while (n-- > 0)
				loc_char('\0', 0);
			break;
		case '\\':
		case '/':	/* pattern matching */
			loc_string(c);
			while (n-- > 1)
				loc_string('\0');
			break;
		default:
			break;
	}

	/* set the cursor for line addresses */
	if (line_addr > 0 ) {
		line_addr = min (line_addr, b_size());
		if (op == ' ')		/* no operator */
			/* move to the first nonwhite character of line */
			b_setline(line_addr);
		else
			/* use position -1 to signify a line address */
			b_setcur(line_addr, -1);
	}

	/* handle the previous location for the ` (backquote) command */
	if (op == ' ') {
		b_getcur(&new_line, &new_pos);
		if (new_line != cur_line || new_pos != cur_pos) {
			prev_line = cur_line;
			prev_pos = cur_pos;
		}
	} else if (op == 'c' || op == 'd')
		/* buffer change; the previous location becomes undefined */
		prev_line = 0;
	/* else op is yank or write; do nothing */
}

/*
--------------------  the j and k commands  --------------------
*
* Entry point:
*
*	do_up_down(i)
*	int i;
*		Move the cursor i lines, staying in the same column. Throughout
*		an uninterrupted sequence of j and k commands, the cursor stays
*		in the same column subject to the constraint that it always lie
*		on a buffer character.
*/

static void do_up_down(i)
int i;
{
	static int col;		/* remembered column */
	int cur_line, cur_pos, new_line, new_pos;

	b_getcur(&cur_line, &cur_pos);
	if (i > 0)
		new_line = min (cur_line + i, b_size());
	else
		new_line = max (cur_line + i, 1);
	/* if the last command was neither j nor k, compute a new column */
	if (k_lastcmd() != 'j' && k_lastcmd() != 'k')
		col = pos_to_col(cur_line, cur_pos);
	/* translate the screen column to a position in the new line */
	new_pos = col_to_pos(new_line, col);
	b_setcur(new_line, new_pos);
}

/* col_to_pos - convert a screen column to a line position */
static int col_to_pos(line, col)
int line, col;
{
	int c, p;
	char text[MAXTEXT-1];

	b_gets(line, text);
	for (c = 1, p = 0; c < col && text[p] != '\0'; ++c, ++p)
		/* keep column c corresponding to position p */
		if (text[p] == '\t')
			while (c%TAB_WIDTH != 0)
				if (++c >= col)
					return(p);
	if (p > 0 && text[p] == '\0')
		--p;
	return(p);
}

/* pos_to_col - convert a line position to a screen column */
static int pos_to_col(line, pos)
int line, pos;
{
	int c, p;
	char text[MAXTEXT-1];

	b_gets(line, text);
	for (c = 1, p = 0; p < pos && text[p] != '\0'; ++c, ++p)
		/* keep column c corresponding to position p */
		if (text[p] == '\t')
			while (c%TAB_WIDTH != 0)
				++c;
	return(c);
}

/*
---------------------  the f, F and ; commands  --------------------
*
* Entry point:
*
*	loc_char(ch, way)
*	char ch;
*	int way;
*		Set cursor to the next position of ch in the current line.
*		If way = 1, then search to the right; otherwise, way = -1 and
*		the search moves left. If ch = '\0', then ch and way are taken
*		from the previous call to loc_char.
*/

static void loc_char(ch, way)
char ch;
int way;
{
	static int w;		/* remembered way */
	int cur_line, cur_pos;
	static char c = 0;	/* remembered ch */
	char *b, buf[MAXTEXT-1];


	if (ch != 0) {
		c = ch;
		w = way;
	} else if (c == 0)
		return;	/* no character specified or remembered */
	b_getcur(&cur_line, &cur_pos);
	b_gets(cur_line, buf);
	for (b = buf + cur_pos + w; b >= buf && *b != '\0'; b += w)
		if (*b == c) {
			b_setcur(cur_line, b - buf);
			break;
		}
}

/*
--------------------  the b and w commands  --------------------
*
* Entry point:
*
*	loc_word(way)
*	int way;
*		Set cursor to the start of the next word.  If way = 1, then
*		search toward file's end; otherwise, search toward file's
*		beginning. Do not "wrap around" at the ends of the file.
*/		

static void loc_word(way)
int way;
{
	int cur_line, cur_pos;
	char *b, buf[MAXTEXT-1];

	b_getcur(&cur_line, &cur_pos);
	b_gets(cur_line, buf);
	/* try the current line */
	for (b = buf + cur_pos + way; b >= buf && *b != '\0'; b += way)
		if (word_start(b, buf))
			break;
	/* try other lines */
	while (!word_start(b,buf)) {
		cur_line += way;
		if (cur_line > b_size() || cur_line < 1)
			break;
		b_gets(cur_line, buf);
		b = (way == 1) ? buf : buf + strlen(buf) - 1;
		while (b >= buf && *b != '\0' && !word_start(b,buf))
			b += way;
		}
	if (word_start(b, buf))
		b_setcur(cur_line, b - buf);
}

/* word_start - tell if s points to the start of a word in text */
static int word_start(s, text)
char *s, *text;
{
	if (s < text || *s == '\0')
		return(0);
	if (s == text)
		return(!isspace(*s));
	if ((ident_char(*s) && !ident_char(s[-1]))
	  || (special_char(*s) && !special_char(s[-1])))
		return(1);
	return(0);
}

/*
--------------------  the /, \ and n commands  --------------------
*
* Entry point:
*
*	loc_string(ch)
*	char ch;
*		Set cursor to the next instance of a user-supplied string.
*		(Loc_string prompts the user to provide the string.)
*		If ch = '/', then the search is forward in the file and
*		wraps around from the end of the file to the start.  If
*		ch = '\', then the search is backward (toward the start of
*		the file) and wraps around from the first line to the last line.
*		For commands /<return> and \<return> the string is taken
*		from the previous call to loc_string.  If ch = '\0', then the
*		string and way are taken from the previous call to loc_string.
*/

static void loc_string(ch)
char ch;
{
	static int way;			/* remembered direction */
	static char string[MAXTEXT-1];	/* remembered pattern */

	int cur_line, cur_pos, first, last, len, line, pos;
	char *pat, out[2], cur_text[MAXTEXT+1], text[MAXTEXT+1];

	if (ch != '\0') {	/* get new pattern and direction */
		way = (ch == '/') ? 1 : -1;
		out[0] = ch;
		out[1] = '\0';
		pat = s_getmsg(out);
		if (*pat == '\b')
			/* user backspaced off left margin */
			return;
		if (*pat != '\0') {
			if (pat[0] == '\\' && pat[1] == 'n')
				*++pat = '\n';
			len = strlen(pat);
			if (len > 1 && pat[len-2] == '\\' && pat[len-1] == 'n') {
				pat[len-2] = '\n';
				pat[len-1] = '\0';
			}
			strcpy(string, pat);
		}
	}
	if (string[0] == '\0')
		/* want to use the old string, but none exists */
		return;

	b_getcur(&cur_line, &cur_pos);
	line = cur_line;
	text[0] = cur_text[0] = '\n';
	/* split the current line at the current position */
	b_gets(cur_line, cur_text+1);
	len = strlen(cur_text);
	cur_text[len] = '\n';	/* newlines were removed when file was read */
	cur_text[len+1] = '\0';
	cur_text[cur_pos+1] = '\0';	/* +1 for leading '\n' */
	if (way > 0) {
		first = cur_pos + 2;
		last = 0;
	} else {
		first = 0;
		last = cur_pos + 2;
	}

	/* search the first section of the current line */
	pos = locate(&cur_text[first], string, way);
	if (pos >= 0)
		pos += first;

	/* if that fails, search the other lines */
	while (pos < 0) {
		if (way > 0)
			line = (line < b_size()) ? line + 1 : 1;
		else
			line = (line > 1) ? line - 1 : b_size();
		if (line == cur_line)
			break;
		b_gets(line, text+1);
		len = strlen(text);
		text[len] = '\n';
		text[len+1] = '\0';
		pos = locate(text, string, way);
	}

	/* if that fails, search the other section of the current line */
	if (pos < 0) {
		line = cur_line;
		pos = locate(&cur_text[last], string, way);
		if (pos >= 0)
			pos += last;
	}

	if (pos >= 0) {	/* found a match */
		--pos;	/* compensate for leading '\n' in text buffer */
		pos = max(pos, 0);	/* if leading '\n' in pattern */
		b_gets(line, text);
		pos = min(pos, (int)strlen(text)-1); /* if matched '\n' after line */
		b_setcur(line, pos);
	}
}

/* locate - return the position of a pattern in a text line */
static int locate(text, pat, way)
char *text, *pat;
int way;
{
	int i, lim;
	char *p, *t;

	if (way > 0) {
		i = 0;
		lim = strlen(text);
	} else {
		i = strlen(text) - 1;
		lim = -1;
	}
	for ( ; i != lim; i += way)
		for (p = pat, t = &text[i]; *p == *t; ++p, ++t)
			if (p[1] == '\0')
				return(i);
	return(-1);	/* no match */
}
