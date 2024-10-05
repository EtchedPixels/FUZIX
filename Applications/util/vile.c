/*
 *	VILE	-	VI Like Editor
 *
 *	Based upon:
 *
 *        ae.c                Anthony's Editor  Mar '92
 *
 *        Public Domain 1991, 1992 by Anthony Howe.  All rights released.
 *
 *	ANSIfied, and extended eventually for Fuzix Alan Cox 2018
 *
 *	Copyright 2018 Alan Cox
 *
 *	Permission is hereby granted, free of charge, to any person obtaining
 *	a copy of this software and associated documentation files (the
 *	"Software"), to deal in the Software without restriction, including
 *	without limitation the rights to use, copy, modify, merge, publish,
 *	distribute, sublicense, and/or sell copies of the Software, and to
 *	permit persons to whom the Software is furnished to do so, subject to
 *	the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included
 *	in all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termcap.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef HUP
#define HUP        "vile.hup"
#endif				/* HUP */

#define MAX_HEIGHT	255

#if defined(__linux__)

/* Linux lacks strlcpy */

size_t strlcpy(char *dst, const char *src, size_t dstsize)
{
  size_t len = strnlen(src, dstsize);
  size_t cp = len >= dstsize ? dstsize - 1 : len;
  memcpy(dst, src, cp);
  dst[cp] = 0;
  return len;
}

#endif

/* ---- */

/* A mini look alike to David Given's libcuss. Actually a bit extended and
   really here so if it's useful to take useful bits from qe we can. It's
   also a good basis to pull out and use for other small apps */

uint_fast8_t screenx = 255, screeny = 255, screen_height, screen_width;

static char *t_go, *t_clreol, *t_clreos;
static uint8_t conbuf[64];
static uint8_t *conp = conbuf;
static mode_t oldperms = 0xFFFF;

extern void con_puts(const char *s);

/* Queue a character to the output buffer */
static int conq(int c)
{
	if (conp == conbuf + sizeof(conbuf)) {
		write(1, conbuf, sizeof(conbuf));
		conp = conbuf;
	}
	*conp++ = (uint8_t) c;
	return 0;
}

/* Make sure the output buffer is written */
void con_flush(void)
{
	write(1, conbuf, conp - conbuf);
	conp = conbuf;
}

static const char hex[] = "0123456789ABCDEF";

/* Put a character to the screen. We handle unprintables and tabs */
void con_putc(uint_fast8_t c)
{
	if (screeny >= screen_height)
		return;
	if (c == '\t') {
		uint8_t n = 8 - (screenx & 7);
		while (n--)
			con_putc(' ');
		return;
	}
	if (c > 127) {
		con_puts("\\x");
		con_putc(hex[c >> 4]);
		con_putc(hex[c & 0x0F]);
		return;
	} else if (c == 127) {
		con_puts("^?");
		return;
	}
	if (c < 32) {
		con_putc('^');
		c += '@';
	}
	conq(c);
	screenx++;
	if (screenx == screen_width) {
		screenx = 0;
		screeny++;
	}
}

/* Write a termcap string out */
static void con_twrite(char *p, int n)
{
#if !defined(__linux__)
	tputs(p, n, conq);
#else
	while (*p)
		conq(*p++);
#endif
}

/* Write a string of symbols including quoting */
void con_puts(const char *s)
{
	uint_fast8_t c;
	while ((c = (uint_fast8_t) * s++) != 0)
		con_putc(c);
}

/* Add a newline */
void con_newline(void)
{
	if (screeny >= screen_height)
		return;
	conq('\n');
	screenx = 0;
	screeny++;
}

/* We need to optimize this but firstly we need to fix our
   tracking logic as we use con_goto internally but don't track
   that verus the true user values */
void con_force_goto(uint_fast8_t y, uint_fast8_t x)
{
	con_twrite(tgoto(t_go, x, y), 2);
	screenx = x;
	screeny = y;
}

void con_goto(uint_fast8_t y, uint_fast8_t x)
{
	if (screenx == x && screeny == y)
		return;
	if (screeny == y && x == 0) {
		conq('\r');
		screenx = 0;
		return;
	}
	if (screeny == y - 1 && x == 0) {
		con_newline();
		return;
	}
	con_force_goto(y, x);
}

/* Clear to end of line */
void con_clear_to_eol(void)
{
	if (screenx == screen_width - 1)
		return;
	if (t_clreol)
		con_twrite(t_clreol, 1);
	else {
		uint_fast8_t i;
		/* Write spaces. This tends to put the cursor where
		   we want it next time too. Might be worth optimizing ? */
		for (i = screenx; i < screen_width; i++)
			con_putc(' ');
		/* otherwise con_newline gets upset */
		con_goto(screeny, screenx);
	}
}

/* Clear to the bottom of the display */

void con_clear_to_bottom(void)
{
	/* Most terminals have a clear to end of screen */
	if (t_clreos)
		con_twrite(t_clreos, screen_height);
	/* If not then clear each line, which may in turn emit
	   a lot of spaces in desperation */
	else {
		uint_fast8_t i;
		for (i = 0; i < screen_height; i++) {
			con_goto(i, 0);
			con_clear_to_eol();
		}
	}
	con_force_goto(0, 0);
}

void con_clear(void)
{
	con_force_goto(0, 0);
	con_clear_to_bottom();
}

int con_scroll(int n)
{
	if (n == 0)
		return 0;
	/* For now we don't do backscrolls: FIXME */
	if (n < 0)
		return 1;
	/* Redraw anyway */
	if (n >= screen_height)
		return 1;
	/* Scrolling down we can do */
	con_force_goto(screen_height - 1, 0);
	while (n--)
		conq('\n');
	con_force_goto(screeny, screenx);
	return 0;
}

/* TODO: cursor key handling */
int con_getch(void)
{
	uint8_t c;
	con_flush();
	if (read(0, &c, 1) != 1)
		return -1;
	return c;
}

int con_size_x(uint8_t c, uint8_t x)
{
	if (c == '\t')
		return 8 - (x & 7);
	/* We will leave unicode out 8) */
	if (c > 127)
		return 4;
	if (c < 32 || c == 127)
		return 2;
	return 1;
}

int con_size(uint8_t c)
{
	return con_size_x(c, screenx);
}

static int do_read(int fd, void *p, int len)
{
	int l;
	if ((l = read(fd, p, len)) != len) {
		if (l < 0)
			perror("read");
		else
			write(2, "short read from tchelp.\n", 25);
		return -1;
	}
	return 0;
}

static char *tnext(char *p)
{
	return p + strlen(p) + 1;
}

static int tty_init(void)
{
	int fd[2];
	pid_t pid;
	int ival[3];
	int status;

	if (pipe(fd) < 0) {
		perror("pipe");
		return -1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		execl("/usr/lib/tchelp", "tchelp", "li#co#cm$ce$cd$cl$", NULL);
		perror("tchelp");
		_exit(1);
	}
	close(fd[1]);
	waitpid(pid, &status, 0);

	do_read(fd[0], ival, sizeof(int));
	if (ival[0] == 0)
		return -1;
	do_read(fd[0], ival + 1, 2 * sizeof(int));

	ival[0] -= 2 * sizeof(int);
	t_go = sbrk((ival[0] + 3) & ~3);

	if (t_go == (void *) -1) {
		perror("sbrk");
		return -1;
	}

	if (do_read(fd[0], t_go, ival[0]))
		return -1;

	close(fd[0]);
	t_clreol = tnext(t_go);
	t_clreos = tnext(t_clreol);
	if (*t_clreos == 0)	/* No clr eos - try for clr/home */
		t_clreos++;	/* cl cap if present */
	if (*t_go == 0) {
		write(2, "Insufficient terminal features.\n", 32);
		return -1;
	}
	/* TODO - screen sizes */
	screen_height = ival[1];
	screen_width = ival[2];
	/* need to try WINSZ and VT ioctls */
	return 0;
}

static struct termios con_termios, old_termios;

void con_exit(void)
{
	tcsetattr(0, TCSANOW, &old_termios);
}

int con_init(void)
{
	static struct winsize w;
#ifdef VTSIZE	
	int n;
#endif	
	if (tty_init())
		return -1;
	if (tcgetattr(0, &con_termios) == -1)
		return -1;
	memcpy(&old_termios, &con_termios, sizeof(struct termios));
	atexit(con_exit);
	con_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	con_termios.c_iflag &= ~(IXON);
	con_termios.c_cc[VMIN] = 1;
	con_termios.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &con_termios) == -1)
		return -1;
#ifdef VTSIZE
	n = ioctl(0, VTSIZE, 0);
	if (n != -1) {
		screen_width = n & 0xFF;
		screen_height = (n >> 8) & 0xFF;
	}
#endif
	if (ioctl(0, TIOCGWINSZ, &w) == 0) {
		screen_width = w.ws_col;
		screen_height = w.ws_row;
	}
	return 0;
}


/* ---- */

typedef struct keytable_t {
	int key;
	int flags;
#define NORPT	1
#define KEEPRPT	2
#define USERPT	4
	int (*func) (void);
} keytable_t;

int done;
int row, col;
/* These are effectively offsets from a pointer so size_t represents
   the largest valid offset. This makes them unsigned so we must be
   careful on comparisons */
size_t indexp, page, epage;
int input;
int repeat = -1;
char *buf;
char *ebuf;
char *gap;
char *egap;
char *filename;
int modified;
int rename_needed;
int status_up;

/* 0 = clean, 1+ = nth char from (1..n) onwards are dirty */
uint8_t dirty[MAX_HEIGHT + 1];
uint8_t dirtyn;

/* For now this is needed by save internally, but we will also want a
   block sized buffer for a few other things later */
char scratch[512];

/* There are lots of cases we touch n + 1, to avoid having to keep checking
   for last lines make this one bigger */

/*
 *        The following assertions must be maintained.
 *
 *        o  buf <= gap <= egap <= ebuf
 *                If gap == egap then the buffer is full.
 *
 *        o  point = ptr(indexp) and point < gap or egap <= point
 *
 *        o  page <= indexp < epage
 *
 *        o  0 <= indexp <= pos(ebuf) <= BUF
 *
 *
 *        Memory representation of the file:
 *
 *                low        buf  -->+----------+
 *                                |  front   |
 *                                | of file  |
 *                        gap  -->+----------+<-- character not in file
 *                                |   hole   |
 *                        egap -->+----------+<-- character in file
 *                                |   back   |
 *                                | of file  |
 *                high        ebuf -->+----------+<-- character not in file
 *
 *
 *        point & gap
 *
 *        The Point is the current cursor position while the Gap is the
 *        position where the last edit operation took place. The Gap is
 *        ment to be the cursor but to avoid shuffling characters while
 *        the cursor moves it is easier to just move a pointer and when
 *        something serious has to be done then you move the Gap to the
 *        Point.
 *
 *
 *        Use of stdio for portability.
 *
 *        Stdio will handle the necessary conversions of text files to
 *        and from a machine specific format.  Things like fixed length
 *        records; CRLF mapping into <newline> (\n) and back again;
 *        null padding; control-Z end-of-file marks; and other assorted
 *        bizare issues that appear on many unusual machines.
 *
 *        AE is meant to be simple in both code and usage.  With that
 *        in mind certain assumptions are made.
 *
 *        Reading:  If a file can not be opened, assume that it is a
 *        new file.  If an error occurs, fall back to a safe state and
 *        assume an empty file.  fread() is typed size_t which is an
 *        unsigned number.  Zero (0) would indicate a read error or an
 *        empty file.  A return value less than BUF is alright, since
 *        we asked for the maximum allowed.
 *
 *        Writing:  If the file can not be opened or a write error occurs,
 *        then we scramble and save the user's changes in a file called
 *        ae.hup.  If ae.hup fails to open or a write error occurs, then
 *        we assume that shit happens.
 *
 *
 *	TODO for Fuzix
 *
 *	Make more vi like
 *	Implement regexps
 *	Yank/paste
 *	Remove stdio and curses use
 *	Use uint8, uint when we can for speed
 *	Use memmove not loops
 *	Macros out of the command blocks we have - may need them all to return
 *	an error status
 *	status bars etc in modeless mode
 *	long line support (including fixing insert_mode)
 *
 *	Can we rewrite most of the editor functions in some kind of bytecode
 *	/ keycode
 *
 *	Can we do async redraw (if you type during redraw we skip updating and
 *	defer the work ?). Need to look at this post curses
 */

int adjust(int, int);
int nextline(int);
int pos(char *);
int prevline(int);
int save(char *);
char *ptr(int);

void warning(const char *);
void dirty_below(void);
void display(int redraw);
void display_line(void);
void movegap(void);
int insertch(char);
int fleft(char);
int fright(char);

int backsp(void);
int bottom(void);
int delete(void);
int delete_line(void);
int down(void);
int insert(void);
int insert_mode(void);
int insert_before(void);
int append_mode(void);
int append_end(void);
int left(void);
int lnbegin(void);
int lnend(void);
int pgdown(void);
int pgup(void);
int pgup_half(void);
int pgdown_half(void);
int redraw(void);
int right(void);
int replace(void);
int quit(void);
int flip(void);
int up(void);
int wleft(void);
int wright(void);
int noop(void);
int digit(void);
int open_after(void);
int open_before(void);
int delete_left(void);
int join(void);
int eword(void);
int findleft(void);
int findright(void);
int zz(void);
int do_goto(void);
int do_del(void);
int do_change(void);
int changeend(void);
int pagetop(void);
int pagemiddle(void);
int pagebottom(void);
int swapchars(void);
int bracket(void);
int colon_mode(void);

#undef CTRL
#define CTRL(x)                ((x) & 0x1f)

/*
 *	Basic vi commands missing (note not all below exactly match vi yet
 *	in their behaviour - much testing is needed).
 *
 *	CTRL-F/CTRL-B	implemented but not exactly as vi
 *	W and S		word skip with punctuation
 *	[] and ()	sentence and paragraph skip
 *	R		replace mode
 *	cw/ce		change word variants
 *	s		substitute
 *	/ and ?		search forward/back
 *	n/N		next search forward/back
 *	u		undo
 *	dw,de		delete word variants
 *	.		repeat last text changing command
 *
 *	Almost all : functionality
 *	All yank/put functionality
 *	All regexps /?nN
 *
 *	Try to write ops as far as possible in terms of each other and a few
 *	non-command 'ops. The goal is to make a lot of this macrocode for size
 *	using the commands plus some extra ops (if x , rpt x, repeat LINES,
 *	repeat LINES/2, repeat rows plus repeat 'difference between cursor
 *	when we started and now' (lets us do d^ d$ etc nicely)
 */

keytable_t table[] = {
#ifdef KEY_LEFT
	{KEY_LEFT, 0, left},
	{KEY_RIGHT, 0, right},
	{KEY_DOWN, 0, down},
	{KEY_UP, 0, up},
#endif
	{27, NORPT, noop},
	{'h', 0, left},
	{'\b', 0, left},
	{'j', 0, down},
	{'\n', 0, down},
	{'k', 0, up},
	{'l', 0, right},
	{' ', 0, right},
	{'b', 0, wleft},
	{'e', 0, eword},
	{CTRL('F'), 0, pgdown},	/* Need D for half screen too */
	{CTRL('B'), 0, pgup},
	{CTRL('U'), 0, pgup_half},
	{CTRL('D'), 0, pgdown_half},
	{'w', 0, wright},
	{'^', NORPT, lnbegin},
	{'$', NORPT, lnend},
	{'G', USERPT, do_goto},	/* Should be 1G */
	{'i', NORPT, insert_mode},
	{'I', NORPT, insert_before},
	{'J', 0, join},
	{'x', 0, delete},
	{'X', 0, delete_left},
	{'o', 0, open_after},
	{'O', 0, open_before},
	{'R', NORPT, redraw},
	{CTRL('L'), NORPT, redraw},
	{'Q', NORPT, quit},
	{'Z', NORPT, zz},
	{'D', 0, delete_line},	/* Should also be dd */
	{'a', NORPT, append_mode},
	{'A', NORPT, append_end},
	{'r', 0, replace},
	{'F', NORPT, findleft},
	{'f', NORPT, findright},
	{'H', NORPT, pagetop},
	{'L', NORPT, pagebottom},
	{'M', NORPT, pagemiddle},
	{'d', 0, do_del},
	{'c', 0, do_change},
	{'C', 0, changeend},
	{'t', 0, swapchars},
	{'%', 0, bracket},
	{'0', KEEPRPT | USERPT, digit},
	{'1', KEEPRPT | USERPT, digit},
	{'2', KEEPRPT | USERPT, digit},
	{'3', KEEPRPT | USERPT, digit},
	{'4', KEEPRPT | USERPT, digit},
	{'5', KEEPRPT | USERPT, digit},
	{'6', KEEPRPT | USERPT, digit},
	{'7', KEEPRPT | USERPT, digit},
	{'8', KEEPRPT | USERPT, digit},
	{'9', KEEPRPT | USERPT, digit},
	{':', NORPT, colon_mode},
	{0, 0, noop}
};


int dobeep(void)
{
	write(1, "\007", 1);
	return 0;
}

char *ptr(int offset)
{
	if (offset < 0)
		return (buf);
	return (buf + offset + (buf + offset < gap ? 0 : egap - gap));
}

int pos(char *pointer)
{
	return (pointer - buf - (pointer < egap ? 0 : egap - gap));
}

int do_goto(void)
{
	if (repeat == -1) {
		epage = indexp = pos(buf);
		return 0;
	}
	/* FIXME: we need to do line tracking really to do this nicely */
	indexp = 0;
	while (repeat-- && !down());
	return 0;
}

int quit(void)
{
	if (!modified)
		done = 1;
	else
		dobeep();
	return 0;
}

int redraw(void)
{
	con_clear();
	display(1);
	return 0;
}

int digit(void)
{
	if (repeat == -1)
		repeat = 0;
	repeat = repeat * 10 + input - '0';
	return 0;
}

void movegap(void)
{
	char *p = ptr(indexp);
	while (p < gap)
		*--egap = *--gap;
	while (egap < p)
		*gap++ = *egap++;
	indexp = pos(egap);
}

int prevline(int offset)
{
	char *p;
	while (buf < (p = ptr(--offset)) && *p != '\n');
	return (buf < p ? ++offset : 0);
}

int nextline(int offset)
{
	char *p;
	while ((p = ptr(offset++)) < ebuf && *p != '\n');
	return (p < ebuf ? offset : pos(ebuf));
}

int adjust(int offset, int column)
{
	char *p;
	int i = 0;
	while ((p = ptr(offset)) < ebuf && *p != '\n' && i < column) {
		i += con_size_x(*p, i);
		++offset;
	}
	return (offset);
}

int left(void)
{
	if (0 < indexp) {
		--indexp;
		return 0;
	}
	return 1;
}

int right(void)
{
	if (indexp < pos(ebuf)) {
		++indexp;
		return 0;
	}
	return 1;
}

int up(void)
{
	indexp = adjust(prevline(prevline(indexp) - 1), col);
	return 0;		/* FIXME */
}

int down(void)
{
	if (indexp == pos(ebuf))
		return 1;
	indexp = adjust(nextline(indexp), col);
	return 0;		/* FIXME */
}

int lnbegin(void)
{
	indexp = prevline(indexp);
	return 0;
}

int lnend(void)
{
	indexp = nextline(indexp);
	return left();		/* FIXME? if on end of last line already ? */
}

/* We want to the top of the next page: in theory I believe we shouldn't
   move at all unless we can move this far. Need to save pointers and test ? */
int pgdown(void)
{
	/* Go to bottom of our page */
	while (row < screen_height) {
		row++;
		down();
	}
	lnbegin();
	page = indexp;
	epage = pos(ebuf);
	return 0;
}

/* Move to the bottom of the previous page, unless cursor is within 1st page */
/* Ditto need to save pointers and test ? */
int pgup(void)
{
	int i = screen_height;
	/* Not quite right but will do for now */
	if (page == 0)
		return 1;
	/* Go to the bottom of the page */
	while (row < screen_height) {
		row++;
		down();
	}
	/* Now go up a page, moving the page marker as we do
	   FIXME: just do the difference!!! */
	while (0 < --i) {
		page = prevline(page - 1);
		up();
	}
	return 0;
}

/* The cursor stays in the same spot and the page moves up, unless we'd hit
   the top in which case the cursor hits the top */
int pgup_half(void)
{
	int i = screen_height / 2;
	while (0 < --i) {
		page = prevline(page - 1);
		up();
	}
	return 0;
}

int pgdown_half(void)
{
	int i = screen_height / 2;
	while (0 < --i) {
		page = nextline(page);
		down();
	}
	return 0;
}

int pagetop(void)
{
	int y = row;
	while (y--)
		up();
	return 0;
}

int pagemiddle(void)
{
	int y = row;
	int t = screen_height / 2;
	while (y < t) {
		down();
		y++;
	}
	while (y-- > t)
		up();
	return 0;
}

int pagebottom(void)
{
	int y = row;
	while (y++ < screen_height - 1)
		down();
	return 0;
}

int swapchars(void)
{
	if (indexp) {
		char *p = ptr(indexp);
		char *q = ptr(indexp - 1);
		char x = *p;

		if (x == *q)
			return 0;
		/* The hard case - moving a newline */
		dirtyn = 1;
		if (x == '\n' || *q == '\n') {
			dirty[row] = 0;
			dirty[row + 1] = 0;
		} else {
			/* FIXME: optimize this for the case where
			   both have the same on screen length */
			dirty[row] = col - 1;
			*p = *q;
			*q = x;
		}
		modified = 1;
		return 0;
	}
	return 1;
}

int wleft(void)
{
	char *p;
	while (!isspace(*(p = ptr(indexp))) && buf < p)
		--indexp;
	while (isspace(*(p = ptr(indexp))) && buf < p)
		--indexp;
	return p == buf;
}

int eword(void)
{
	char *p;
	while (!isspace(*(p = ptr(indexp))) && p < ebuf)
		++indexp;
	return p == ebuf;
}

int wright(void)
{
	char *p;
	eword();
	while (isspace(*(p = ptr(indexp))) && p < ebuf)
		++indexp;
	return p == ebuf;
}

int fleft(char c)
{
	char *p;
	if (*(p = ptr(indexp)) == c && buf < p)
		--indexp;
	while (*(p = ptr(indexp)) != c && buf < p)
		--indexp;
	return p == buf;
}

int fright(char c)
{
	char *p;
	if (*(p = ptr(indexp)) == c && p < ebuf)
		++indexp;
	while (*(p = ptr(indexp)) != c && p < ebuf)
		++indexp;
	return p == ebuf;
}

int findleft(void)
{
	int c = con_getch();
	if (c < 0 || c > 255)
		return 1;
	return fleft(c);
}

int findright(void)
{
	int c = con_getch();
	if (c < 0 || c > 255)
		return 1;
	return fright(c);
}

/* Does it make sense to merge these with fleft/fright ? */
int findpair(char in, char out, char dir)
{
	unsigned int depth = 0;

	while (1) {
		char *p = ptr(indexp);
		char c = *p;
		if (c == in)
			depth++;
		if (c == out) {
			if (--depth == 0)
				return 0;
		}
		if (dir == -1) {
			if (indexp == 0)
				return 1;
			indexp--;
		} else {
			if (p == ebuf)
				return 1;
			indexp++;
		}
	}
}


/* Real vi doesn't match < > but it's two bytes cost to add and really rather
   useful */
static const char brackettab[] = "([{<)]}>";

int bracket(void)
{
	char c = *ptr(indexp);
	int ip = indexp;
	char *x = strchr(brackettab, c);

	if (x == NULL)
		return 1;

	if (x < brackettab + 4) {
		if (findpair(*x, x[4], 1) == 0)
			return 0;
		indexp = ip;
		dobeep();
		return 1;
	} else {
		if (findpair(*x, x[-4], -1) == 0)
			return 0;
		indexp = ip;
		dobeep();
		return 1;
	}
}

/* Do we need a filter on this and insert_mode ? */
int insertch(char ch)
{
	movegap();
	if (gap < egap) {
		*gap++ = ch == '\r' ? '\n' : ch;
		dirtyn = 1;
		if (ch == '\r' || ch == '\n')
			dirty_below();
		dirty[row] = col;
		indexp = pos(egap);
		modified = 1;
		return 0;
	}
	return 1;
}

int insert(void)
{
	return insertch(input);
}

int insert_before(void)
{
	lnbegin();
	return insert_mode();
}

/* Need to do a replace mode version */
int insert_mode(void)
{
	int ch;
	movegap();
	/* Get the cursor in the right spot */
	display(0);
	con_flush();
	while ((ch = con_getch()) != 27) {
		if (ch == 0)
			continue;
		if (ch == '\f' || ch == -1)
			break;
		if (ch == '\b' || ch == 127) {
			/* There is a hard case here where we delete
			   a newline */
			if (*gap == '\n') {
				dirty[row] = 0;
				dirty_below();
				dirtyn = 1;
			}
			if (buf < gap)
				--gap;
		} else if (gap < egap) {
			*gap++ = ch == '\r' ? '\n' : ch;
			dirty[row] = 0;
			if (ch == '\n')
				dirty_below();
			dirtyn = 1;
			modified = 1;
		}
		indexp = pos(egap);
		display_line();
	}
	return 0;
}

int append_mode(void)
{
	if (!right())
		return insert_mode();
	return 0;
}

int append_end(void)
{
	lnend();
	return insert_mode();
}

int replace(void)
{
	int c = con_getch();
	/* FIXME: saner filter ? */
	if (c < 0 || c > 255)
		return 1;
	if (!delete())
		return insertch(c);
	return 0;
}

static int do_delete_line(void)
{
	movegap();
	while (egap < ebuf && *egap != '\n')
		indexp = pos(++egap);
	return delete();
}

int delete_line(void)
{
	lnbegin();
	do_delete_line();
	modified = 1;
	dirty_below();
	dirty[row] = 0;
	dirtyn = 1;
	return 0;
}

int backsp(void)
{
	movegap();
	if (buf < gap) {
		--gap;
		indexp = pos(egap);
		return 0;
	}
	return 1;
}

int delete(void)
{
	movegap();
	if (egap < ebuf) {
		modified = 1;
		if (*egap == '\n')
			dirty_below();
		dirty[row] = col;
		dirtyn = 1;
		indexp = pos(++egap);
		return 0;
	}
	return 1;
}

int delete_left(void)
{
	if (!left())
		return delete();
	return 1;
}

int do_del(void)
{
	int c = con_getch();
	if (c == '$')		/* Delete to end */
		return do_delete_line();
	else if (c == 'd') {
		return delete_line();
	} else if (c == '^') {
		while (indexp && *ptr(indexp) != '\n' && !delete_left());
		return 0;
	} else {
		dobeep();
		return 1;
	}
	/* TODO dw and de */
}

int do_change(void)
{
	if (!do_del())
		return insert_mode();
	return 1;
}

int changeend(void)
{
	if (!delete_line())
		return insert_mode();
	return 1;
}

int join(void)
{
	lnend();
	if (egap != ebuf)
		return delete();
	return 1;
}

int open_before(void)
{
	lnend();
	if (!insertch('\n'))
		return append_mode();
	return 1;
}

int open_after(void)
{
	lnbegin();
	if (!insertch('\n') && !up())
		return append_mode();
	return 0;
}

int save(char *fn)
{
	int fd;
	int i;
	size_t length;
	char *gptr;
	mode_t mode;

	strlcpy(scratch, fn, 511);
	gptr = scratch + strlen(scratch);
	*gptr++ = '~';
	*gptr = 0;

	/* Rename the original but not more working copies */
	if (rename_needed) {
		/* Rename the old file - if it exists */
		unlink(scratch);
		if (link(fn, scratch) == 0) {
			if (unlink(fn))
				return 0;
		} else if (errno != ENOENT)
			return 0;
		rename_needed = 0;
	}

	/* Open file: if no permissions to set use rw/rw/rw + umask
	   if not force it, but take care to start private */
	if  (oldperms == 0xFFFF)
		mode = 0666;
	else
		mode = 0600;
	if ((fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0)
		return 0;
	if (oldperms != 0xFFFF)
		fchmod(fd, oldperms & 0777);
	i = indexp;
	indexp = 0;
	movegap();
	gptr = egap;
	length = (size_t) (ebuf - egap);
	/* Write out in chunks so we are ok on 16bit int 32bit long */
	while(length) {
		size_t l = length;
		if (l > 8192)
			l = 8192;
		if (write(fd, gptr, l) != l)
			break;
		length -= l;
		gptr += l;
	}
	indexp = i;
	if (close(fd) || length)
		return 0;
	modified = 0;
	return 1;
}

int save_done(char *path, uint8_t n)
{
	/* Check if changed ? */
	if (!save(path))
		warning(strerror(errno));
	else
		done = n;
	return 1;
}

int zz(void)
{
	int c = con_getch();
	if (c != 'Z' && c != 'z') {
		dobeep();
		return 0;
	}
	if (*filename)
		return save_done(filename, 1);
	else
		warning("no filename");
}

int noop(void)
{
	return 0;
}

void status_wipe(void)
{
	dirty[screen_height - 1] = 0;
	display(0);
	status_up = 0;
}

/*
 *	We need to emulate a minimal subset of commands people habitually use
 *
 *	:q :q!
 *	:x
 *
 *	:w :w! (with path)
 *	:r (maybe)
 *	:number
 *	:s/
 *	:e file
 *	:!shell
 *	!!
 */
static void colon_process(char *buf)
{
	int quit = 0;
	if (*buf == 'q') {
		if (!modified || buf[1] == '!')
			done = 1;
		else
			warning("No write since last change.");
		return;
	}
	if (*buf == 'x') {
		save_done(filename, 1);
		return;
	}
	if (*buf == 'w') {
		buf++;
		if (*buf == 'q') {
			quit = 1;
			buf++;
		}
		if (*buf == ' ') {
			rename_needed = 1;
			save_done(buf + 1, quit);
			/* FIMXE: should change filename */
		} else if (*buf == 0)
			save_done(filename, quit);
		else
			warning("unknown :w option.");
		return;
	}
	if (isdigit(*buf)) {
		repeat = atoi(buf);
		do_goto();
		return;
	}
	warning("unknown : command.");
}

int colon_mode(void)
{
	char buf[132];
	char *bp = buf;
	int c;
	int xp = 1;	/* The : */

	/* Wipe the status line and prompt */
	con_goto(screen_height - 1, 0);
	con_putc(':');
	con_clear_to_eol();
	con_goto(screen_height - 1, 1);

	*bp = 0;
	while (1) {
		c = con_getch();
		if (c < 0 || c > 255 || c == 27) {
			dobeep();
			break;
		}
		if (c == '\n' || c == '\r') {
			colon_process(buf);
			break;
		}
		/* TAB is hard for erase handling so skip it */
		if (c == '\t') {
			dobeep();
			continue;
		}
		/* Erase as many symbols as the character took */
		if (c == 8 || c == 127) {
			if (bp != buf) {
				/* This doesn't work for tab but we avoided
				   tab above. Other symbols are fixed width */
				uint8_t s = con_size(*--bp);
				xp -= s;
				con_goto(screen_height - 1, xp);
				if (t_clreol)
					con_clear_to_eol();
				else {
					while (s--)
						con_putc(' ');
					con_goto(screen_height - 1, xp);
				}
				*bp = 0;
			}
		} else {
			uint8_t s = con_size(c);
			/* Never print in the bottom right corner */
			if (bp < buf + 130 && screenx + s < screen_width - 2) {
				*bp++ = c;
				*bp = 0;
				con_putc(c);
				xp += s;
			}
		}
	}
	/* Clean the status line */
	status_wipe();
	return 0;
}

void warning(const char *p)
{
	/* FIXME: This sort of assumes the error fits one line */
	/* Ideally inverse video etc and clr to end */
	con_goto(screen_height - 1, 0);
	con_puts(p);
	con_clear_to_eol();
	dobeep();
	status_up = 1;
}

/*
 *	Mark the lower screen area dirty
 */

void dirty_below(void)
{
	memset(dirty + row, 0, MAX_HEIGHT - (row + 1));
	dirtyn = 1;
}

void adjust_dirty(int n)
{
	if (n < 0) {
		memmove(dirty, dirty -  n, MAX_HEIGHT + n);
		memset(dirty + MAX_HEIGHT + n, 0, -n);
	} else if (n > 0) {
		memmove(dirty + n, dirty, MAX_HEIGHT - n);
		memset(dirty, 0, n);
	}
}

void dirty_all(void)
{
	memset(dirty, 0, MAX_HEIGHT);
	dirtyn = 1;
}

/*
 *	The main redisplay logic.
 */

static unsigned tilde_start;	/* First line already set to ~ for end of doc */

void display(int redraw)
{
	char *p;
	unsigned int i, j;
	int opage = 0;
	uint_fast8_t inpos = 0;
	uint8_t *dirtyp = dirty;

	if (indexp < page) {
		/* We don't yet do backscrolls */
		redraw = 1;
		page = prevline(indexp);
	}
	/* Need to move down */
	if (epage <= indexp) {
		page = nextline(indexp);
		i = page == pos(ebuf) ? screen_height - 2 : screen_height;
		while (0 < i--) {
			opage++;
			page = prevline(page - 1);
		}
		/* Try and scroll the screen */
		if (con_scroll(opage))
			adjust_dirty(opage);
		else
			redraw = 1;
	}

	if (redraw) {
		dirtyn = 1;
		dirty_all();
		tilde_start = screen_height;
	}

	i = j = 0;
	epage = page;

	/*
	 *      We need to add two optimized paths to this
	 *      1. Only the cursor moved
	 *      2. Only the current line changed
	 *
	 *      We should also hardcode this to use direct pointer
	 *      iteration over the two buffers with a simple state machine
	 */
	while (1) {
		/* We have found the cursor position of the insert point */
		if (indexp == epage) {
			row = i;
			col = j;
			/* No updating needed fast path out */
			if (dirtyn == 0) {
				con_goto(i, j);
				return;
			}
		}
		p = ptr(epage);
		/* We ran out of screen or buffer */
		if (i >= screen_height || ebuf <= p)
			break;
		/* Normal characters */
		if (*p != '\n') {
			uint_fast8_t s = con_size_x(*p, j);
			/* If the symbol fits and is beyond our dirty marker */
			if (j >= *dirtyp && j + s < screen_width) {
				/* Move cursor only if needed. We assume
				   con_goto will do any optimizing */
				if (!inpos) {
					con_goto(i, j);
					inpos = 1;
				}
				/* Draw the symbol */
				con_putc(*p);
			}
			j += s;
		} else {
			/* A newline, wipe to the end of this line and
			   mark it clean, move on to the next */
			if (*dirtyp != 255) {
				if (!inpos)
					con_goto(i, j);
				con_clear_to_eol();
				*dirtyp = 255;
			}
			++i;
			j = 0;
			inpos = 0;
			dirtyp++;
		}
		++epage;
	}
	/* Clear the end of our final line */
	if (j && *dirtyp != 255) {
		if (!inpos)
			con_goto(i, j);
		con_clear_to_eol();
		*dirtyp = 255;
		dirtyp++;
		i++;
	}
	/* Now mark out the unused lines with ~ markers if needed */
	j = i + 1;
	while (++i < tilde_start) {
		if (*dirtyp != 255) {
			*dirtyp = 255;
			con_goto(i, 0);
			con_putc('~');
			con_clear_to_eol();
		}
		dirtyp++;
	}
	tilde_start = j;
	/* Put the cursor back where the user expects it to be */
	con_goto(row, col);
}

void display_line(void)
{
	dirty[row] = col;
	display(0);
}

void hupped(int sig)
{
	save_done(HUP, 0);
	_exit(0);		/* Paranoia */
}

void oom(void)
{
	write(2, "out of memory.\n", 15);
	exit(1);
}

static int doread(const char *name, int fd, char *ptr, int size)
{
	int n = read(fd, ptr, size);
	if (n == -1) {
		perror(name);
		exit(2);
	}
	return n;
}

int main(int argc, char *argv[])
{
	int fd;
	uint8_t mem;
	uint8_t i;

	if (sizeof(void *) == 2) {
		buf = sbrk(0);
		ebuf = (char *)&mem - 768;
		if (ebuf < buf || brk(ebuf))
			oom();
	} else {
		buf = malloc(65535);
		ebuf = buf + 65535;
		if (buf == NULL) {
			buf = malloc(32768);
			ebuf = buf + 32768;
			if (buf == NULL)
				oom();
		}
	}

	gap = buf;
	egap = ebuf;
	if (argc < 2)
		filename = "";
	else {
		struct stat t;
		fd = open(filename = *++argv, O_RDONLY);
		if (fd == -1 && errno != ENOENT) {
			perror(*argv);
			return 2;
		}
		if (fd != -1) {
			size_t size;
			size_t o = 0;
			int n = 0;
			size = ebuf - buf;
			/* We can have 32bit ptr, 16bit int even in theory */
			if (size >= 8192) {
				while ((n = doread(*argv, fd, buf + o, 8192)) > 0) {
					gap += n;
					size -= n;
					o += n;
				}
			} else
				n = doread(*argv, fd, buf + o, size);
			if (n < 0) {
				perror(*argv);
				return 2;
			}
			gap += n;
			if (fstat(fd, &t) == 0)
				oldperms = t.st_mode;
			close(fd);
		}
		rename_needed = 1;
	}

	if (con_init())
		return (3);

	con_goto(0,0);
	
	signal(SIGHUP, hupped);
	do_goto();
	display(1);
	repeat = -1;
	while (!done) {
		display(0);
		i = 0;
		input = con_getch();
		if (status_up)
			status_wipe();
		while (table[i].key != 0 && input != table[i].key)
			++i;
		if (repeat < 2 || (table[i].flags & (NORPT | USERPT)))
			(*table[i].func) ();
		else
			while (repeat--)
				(*table[i].func) ();
		if (!(table[i].flags & KEEPRPT))
			repeat = -1;
	}
	con_goto(screen_height - 1, 0);
	con_clear_to_eol();
	con_newline();
	con_flush();
	return (0);
}
