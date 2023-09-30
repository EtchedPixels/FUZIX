/*
 *	Fleamacs
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
#define HUP        "fleamacs.hup"
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

uint_fast8_t screenx, screeny, screen_height, screen_width;

static char *t_go, *t_clreol, *t_clreos, *t_me, *t_mr;
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
void con_putc(uint8_t c)
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
	uint8_t c;
	while ((c = (uint8_t) * s++) != 0)
		con_putc(c);
}

void con_putsn(const char *s, unsigned n)
{
	uint8_t c;
	while (n-- && (c = (uint8_t) * s++) != 0)
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
	con_goto(0, 0);
	con_clear_to_bottom();
}

void con_reverse(void)
{
#ifndef __linux__
	con_twrite(t_mr, 1);
#endif
}

void con_normal(void)
{
#ifndef __linux__
	con_twrite(t_me, 1);
#endif
}

int con_scroll(int n)
{
	if (n == 0)
		return 0;
	/* For now we don't do backscrolls: FIXME */
	if (n < 0)
		return 1;
	/* This is a redraw anyway */
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
	t_me = tnext(t_clreos);
	t_mr = tnext(t_me);
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
static int con_live;

void con_exit(void)
{
	tcsetattr(0, TCSANOW, &old_termios);
	con_live = 0;;
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
	con_live = 1;
	return 0;
}


/* ---- */

typedef struct keytable_t {
	int key;
	int flags;
	int (*func) (void);
} keytable_t;

int done;
int row, col;
/* These are effectively offsets from a pointer so size_t represents
   the largest valid offset. This makes them unsigned so we must be
   careful on comparisons */
size_t indexp, page, epage, mark;
int input;
int repeat = 1;
int keymode;
int quoted;
char *buf;
char *ebuf;
char *gap;
char *egap;
char *filetail;
int modified;
int rename_needed;
int status_up;
int status_dirty;

/* 0 = clean, 1+ = nth char from (1..n) onwards are dirty */
uint8_t dirty[MAX_HEIGHT + 1];
uint8_t dirtyn;

/* Need to find a better way */
char filepath[512];

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
 *	Repeat counts
 *	Remove a few bits no longer used
 *	Cursor keys
 *	Yank/paste
 *	Use uint8, uint when we can for speed
 *	Use memmove not loops
 *	Macros out of the command blocks we have - may need them all to return
 *	an error status
 *	long line support (including fixing insert_mode)
 *
 *	Can we rewrite most of the editor functions in some kind of bytecode
 *	/ keycode
 *
 *	Can we do async redraw (if you type during redraw we skip updating and
 *	defer the work ?). Need to look at this post curses
 */

void dirty_all(void);
int adjust(int, int);
int nextline(int);
int pos(char *);
int prevline(int);
int save(const char *);
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
int insert_nl(void);
int insert_tab(void);
int insert_space_r(void);
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
int wupper(void);
int wcaps(void);
int wlower(void);
int noop(void);
int digit(void);
int digit4(void);
int open_after(void);
int open_before(void);
int delete_left(void);
int join(void);
int eword(void);
int findleft(void);
int findright(void);
int do_goto(void);
int top(void);
int bottom(void);
int swapchars(void);
int enter_meta(void);
int enter_ctrlx(void);
int enter_quoted(void);
int filename(void);
int save_exit(void);
int save_only(void);
int load(void);
int set_mark(void);
int swap_mark(void);
int bang_shell(void);
int run_shell(void);
int searchf(void);
int searchr(void);
int wipe(void);

char *command_prompt(const char *cmd);

#undef CTRL
#define CTRL(x)                ((x) & 0x1f)

/*
 *	Status:
 *	Moving cursor:
 *		Need to add M-N M-P (paragraph)
 *	Deleting and inserting:
 *		Done
 *	More deleting/inserting:
 *		M-^W ^W M-sp M-W ^X^O ^O
 *	Searching:
 *		^XS ^XR
 *	Replacing:
 *		M-R M-^R	(need region support)
 *	Capitalizing/Transposing
 *		^X^L ^X^U ^Q	(need region support)
 *	Regions
 *		Done
 *	Copying and Moving
 *		^Y ESC-W
 *	Formatting
 *		^XF M-Tab M-Q ^X= M-^C
 *	Reading From Disk
 *		^X^F ^X^I ^X^V
 *	Saving To Disk
 *		Done
 *	Accessing the OS
 *		Done (for relevant stuff)
 *	Special Keys
 *		^U M-n M-X (won't do)
 *
 *	Need to do tmp file for yank buffer
 *
 *	Try to write ops as far as possible in terms of each other and a few
 *	non-command 'ops. The goal is to make a lot of this macrocode for size
 *	using the commands plus some extra ops (if x , rpt x, repeat LINES,
 *	repeat LINES/2, repeat rows plus repeat 'difference between cursor
 *	when we started and now' (lets us do d^ d$ etc nicely)
 */

#define META	0x4000
#define MX	0x2000

#define NORPT	1
#define KEEPRPT	2
keytable_t table[] = {
#ifdef KEY_LEFT
	{KEY_LEFT, 0, left},
	{KEY_RIGHT, 0, right},
	{KEY_DOWN, 0, down},
	{KEY_UP, 0, up},
#endif
	{CTRL('B'), 0, left},
	{'\b', 0, delete_left},
	{127, 0, delete_left},
	{CTRL('N'), 0, down},
	{CTRL('P'), 0, up},
	{CTRL('F'), 0, right},
	{META | 'b', 0, wleft},
	{META | '\b', 0, eword},
	{CTRL('Z'), 0, pgup_half},
	{CTRL('V'), 0, pgdown_half},
	{META |'F', 0, wright},
	{META |'U', 0, wupper},
	{META |'C', 0, wcaps},
	{META |'L', 0, wlower},
	{CTRL('A'), NORPT, lnbegin},
	{CTRL('E'), NORPT, lnend},
	{META | 'G', 0, do_goto},
	{META | '<', 0, top},
	{META | '>', 0, bottom},
	{CTRL('D'), 0, delete},
	{CTRL('L'), NORPT, redraw},
	{CTRL('K'), 0, delete_line},	/* Should also be dd */
	{MX | CTRL('C'), NORPT, quit},
	{MX | 'N', NORPT, filename},
	{MX | CTRL('R'), NORPT, load},
	{MX | CTRL('S'), NORPT, save_only},
	{MX | CTRL('W'), NORPT, save_only},
	{META | CTRL('\\'), NORPT, save_only},
	{MX|CTRL('C'), NORPT, quit},
	{META|'Z', NORPT, save_exit},
	{CTRL('C'), 0, insert_space_r},
	{CTRL('T'), 0, swapchars},
	{META | ' ', 0, set_mark},
	{MX|CTRL('X'), 0, swap_mark},
	{MX|'!', 0, bang_shell},
	{MX|'C', 0, run_shell},
	{META | '0', KEEPRPT | NORPT, digit},
	{META | '1', KEEPRPT | NORPT, digit},
	{META | '2', KEEPRPT | NORPT, digit},
	{META | '3', KEEPRPT | NORPT, digit},
	{META | '4', KEEPRPT | NORPT, digit},
	{META | '5', KEEPRPT | NORPT, digit},
	{META | '6', KEEPRPT | NORPT, digit},
	{META | '7', KEEPRPT | NORPT, digit},
	{META | '8', KEEPRPT | NORPT, digit},
	{META | '9', KEEPRPT | NORPT, digit},
	{CTRL('U'), KEEPRPT | NORPT, digit4},
	{27, NORPT, enter_meta},
	{CTRL('X'), NORPT, enter_ctrlx },
	{CTRL('Q'), NORPT, enter_quoted },
	{MX | CTRL('X'), 0, noop },	/* Cancel X- mode */
	{'\t', 0, insert_tab },
	{'\n', 0, insert_nl },
	{'\r', 0, insert_nl },
	{ CTRL('S'), 0, searchf },
	{ CTRL('R'), 0, searchr },
	{ CTRL('W'), 0, wipe },
	{0, 0, NULL }
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
	char *p = command_prompt("Line: ");
	unsigned n;
	if (p == NULL)
		return 1;
	n = atoi(p);
	top();
	while(n-- && down() == 0);
	return 0;
}

int top(void)
{
	epage = indexp = 0;
	return 0;
}

int bottom(void)
{
	epage = indexp = pos(ebuf);
	return 0;
}

int writeout(int final)
{
	char *p;
	if (!modified)
		done = final;
	else {
		p = command_prompt("Save changes Y/N: ");
		if (!p) {
			dobeep();
			return 1;
		}
		*p = toupper(*p);
		if (*p == 'Y')
			done = save(filepath);
		else if (*p == 'N')
			done = final;
		else {
			dobeep();
			return 1;
		}
	}
	return 0;
}

int quit(void)
{
	return writeout(1);
}

int redraw(void)
{
	con_clear();
	display(1);
	return 0;
}

int digit(void)
{
	repeat = (input & 0xFF) - '0';
	if (repeat == 0)
		repeat = 10;
	return 0;
}

int digit4(void)
{
	repeat = 4;
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
	char *p = buf;
	while (offset && buf < (p = ptr(--offset)) && *p != '\n');
	return (buf < p ? ++offset : 0);
}

int nextline(int offset)
{
	char *p;
	while ((p = ptr(offset)) < ebuf && *p != '\n')
		offset++;
	return (p < ebuf ? offset + 1 : pos(ebuf));
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
	if (indexp == 0)
		return 1;
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
//		page = prevline(page - 1);
		up();
	}
	return 0;
}

int pgdown_half(void)
{
	int i = screen_height / 2;
	while (0 < --i) {
//		page = nextline(page);
		down();
	}
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
			dirty[row] = 255;
			dirty[row + 1] = 255;
		} else {
			/* FIXME: optimize this for the case where
			   both have the same on screen length */
			dirty[row] = col;
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

int espace(void)
{
	char *p;
	while (isspace(*(p = ptr(indexp))) && p < ebuf)
		++indexp;
	return p == ebuf;
}

int wright(void)
{
	eword();
	return espace();
}

int wupper(void)
{
	char *p;
	espace();
	while (!isspace(*(p = ptr(indexp))) && p < ebuf) {
		*p = toupper(*p);
		++indexp;
	}
	return p == ebuf;
}

int wcaps(void)
{
	char *p;
	espace();
	if(!isspace(*(p = ptr(indexp))) && p < ebuf) {
		*p = toupper(*p);
		++indexp;
	}
	return eword();
}

int wlower(void)
{
	char *p;
	espace();
	while (!isspace(*(p = ptr(indexp))) && p < ebuf) {
		*p = tolower(*p);
		++indexp;
	}
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

int insertch(char ch)
{
	movegap();
	if (gap < egap) {
		*gap++ = ch;
		dirtyn = 1;
		if (ch == '\n')
			dirty_below();
		dirty[row] = col;
		indexp = pos(egap);
		modified = 1;
		return 0;
	}
	return 1;
}

int insert_tab(void)
{
	return insertch('\t');
}

int insert_nl(void)
{
	return insertch('\n');
}

int insert_space_r(void)
{
	insertch(' ');
	return left();
}

static int do_delete_line(void)
{
	movegap();
	while (egap < ebuf && *egap != '\n')
		indexp = pos(++egap);
	return 0;
}

int delete_line(void)
{
	lnbegin();
	do_delete_line();
	modified = 1;
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

int set_mark(void)
{
	mark = indexp;
	warning("Mark set");
	return 0;
}

int swap_mark(void)
{
	size_t tmp = mark;
	mark = indexp;
	indexp = tmp;
	return 0;
}

/* Naiive but small algorithm */
static char *memmem_b(const char *buf, const char *ebuf, const char *find, size_t flen)
{
	const char *p = ebuf - flen;
	while(p >= buf) {
		if (memcmp(p, find, flen) == 0)
			return (char *)p;
		p--;
	}
	return NULL;
}

static char *memmem(const char *buf, const char *ep, const char *find, size_t flen)
{
	const char *p = buf;
	ep -= flen;
	while(p <= ep) {
		if (memcmp(p, find, flen) == 0)
			return (char *)p;
		p++;
	}
	return NULL;
}

int searchf(void)
{
	char *p = command_prompt("Search:");
	if (p == NULL)
		return 1;
	movegap();
	/* The search area is now linear from egap to ebuf */
	p = memmem(egap, ebuf, p, strlen(p));
	if (p == NULL) {
		warning("Not found");
		return 1;
	}
	indexp = pos(p);
	return 0;
}


int searchr(void)
{
	char *p = command_prompt("Search:");
	if (p == NULL)
		return 1;
	movegap();
	/* The search area is now linear from start to gap */
	p = memmem_b(buf, gap, p, strlen(p));
	if (p == NULL) {
		warning("Not found");
		return 1;
	}
	indexp = pos(p);
	return 0;
}

/* Need a shared helper as "M-W is wipe but writing the block to the scratch
   yank file and insert file and yankback are identical */
int wipe(void)
{
	size_t bias = indexp - mark;
	modified = 1;
	/* Wipes from mark to cursor. See if that does anything */
	if (indexp < mark)
		return 1;
	/* New cursor position (at the mark) */
	indexp = mark;
	movegap();
	/* The data to kill is now from egap upwards for bias */
	egap += bias;
	dirty[row] = 255;	/* Optimize later */
	dirty_below();
	return 0;
}

int bang_shell(void)
{
	char *p = command_prompt("Command");
	if (p == NULL)
		return 1;
	system(p);
	return 0;
}

int run_shell(void)
{
	/* Probably should fork/execve this explicitly */
	char *p = getenv("SHELL");
	if (p == NULL)
		p = "/bin/sh";
	tcsetattr(0, TCSADRAIN, &old_termios);
	system(p);
	tcsetattr(0, TCSADRAIN, &con_termios);
	con_getch();
	return 0;
}

int enter_meta(void)
{
	keymode = META;
	warning("M-");
	return 0;
}

int enter_ctrlx(void)
{
	keymode = MX;
	warning("^X-");
	return 0;
}

int enter_quoted(void)
{
	quoted = 1;
	warning("Quote-");
	return 0;
}

int save(const char *path)
{
	int fd;
	int i;
	size_t length;
	mode_t mode;
	char *gptr;

	/* TODO safe saving with backup */

	/* Open file: if no permissions to set use rw/rw/rw + umask
	   if not force it, but take care to start private */
	if  (oldperms == 0xFFFF)
		mode = 0666;
	else
		mode = 0600;
	if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0)
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

int save_done(const char *path, uint8_t n)
{
	/* Check if changed ? */
	if (!save(path))
		warning(strerror(errno));
	else
		done = n;
	return 1;
}

int save_only(void)
{
	if (!save(filepath)) { 
		warning(strerror(errno));
		return 1;
	}
	return 0;
}

int save_exit(void)
{
	return save_done(filepath, 1);
}

int noop(void)
{
	return 0;
}

void status_wipe(void)
{
	con_goto(screen_height - 1, 0);
	con_clear_to_eol();
	status_up = 0;
}

static void file_set_tail(void)
{
	filetail = strrchr(filepath, '/');
	if (filetail == NULL)
		filetail = filepath;
	else
		filetail++;
}

int filename(void)
{
	char *p = command_prompt("Name: ");
	if (p) {
		strlcpy(filepath, p, 511);
		file_set_tail();
		status_dirty = 1;
		return 0;
	}
	return 1;
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

static int loadfile(char *p)
{
	struct stat t;
	int fd;

	gap = buf;
	egap = ebuf;

	dirty_all();

	strlcpy(filepath, p, 511);
	fd = open(filepath, O_RDONLY);
	if (fd == -1 && errno != ENOENT)
		goto bad;
	if (fd != -1) {
		size_t size;
		size_t o = 0;
		int n = 0;
		size = ebuf - buf;
		/* We can have 32bit ptr, 16bit int even in theory */
		if (size >= 8192) {
			while ((n = doread(p, fd, buf + o, 8192)) > 0) {
				gap += n;
				size -= n;
				o += n;
			}
		} else
			n = doread(p, fd, buf + o, size);
		if (n < 0)
			goto bad;
		gap += n;
		if (fstat(fd, &t) == 0)
			oldperms = t.st_mode;
		close(fd);
	}
	rename_needed = 1;
	return 0;

bad:
	if (con_live == 0)
		perror(p);
	else
		warning(strerror(errno));
	return 1;

}

int load(void)
{
	writeout(0);
	if (filename())
		return 1;
	loadfile(filepath);
	top();
	return 0;
}

/*
 *	Prompt for input on the bottom line
 */
char *command_prompt(const char *x)
{
	static char buf[132];
	char *bp = buf;
	int c;
	int xp = strlen(x);	/* Assume no funny chars */

	/* Wipe the status line and prompt */
	con_goto(screen_height - 1, 0);
	con_puts(x);
	con_clear_to_eol();
	con_goto(screen_height - 1, xp);

	*bp = 0;
	while (1) {
		c = con_getch();
		if (c < 0 || c > 255 || c == CTRL('G')) {
			status_wipe();
			return NULL;
		}
		if (c == '\n' || c == '\r')
			break;

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
	return buf;
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
	memset(dirty, 0, MAX_HEIGHT - (row + 1));
	dirtyn = 1;
}

void adjust_dirty(int n)
{
	if (n < 0) {
		memmove(dirty, dirty - n, MAX_HEIGHT + n);
		memset(dirty + MAX_HEIGHT + n, 0, -n);
	} else if (n > 0) {
		memmove(dirty + n, dirty, MAX_HEIGHT - n);
		memset(dirty, 0, n);
	}
}

void dirty_all(void)
{
	memset(dirty, 255, MAX_HEIGHT);
	dirtyn = 1;
}

/*
 *	The main redisplay logic.
 */

static unsigned cl_start;	/* Track clear lines below bottom of edit */

void display(int redraw)
{
	char *p;
	unsigned int i, j;
	int opage = page;
	uint_fast8_t inpos = 0;
	uint8_t *dirtyp = dirty;

	if (indexp < page)
		page = prevline(indexp);
	if (epage <= indexp) {
		page = nextline(indexp);
		i = page == pos(ebuf) ? screen_height - 2 : screen_height - 2;
		while (0 < i--)
			page = prevline(page - 1);
	}

	/* opage is the delta so we know if we are going to scroll. If it's
	   negative then we need to reverse scroll, if its positive we need
	   to normal scroll */
	opage -= page;

	/* If we can't scroll this then redraw the lot */
	/* TODO: see if its worth doing a scroll and repaint for the lower
	   lines */
	if (1 || (opage && con_scroll(opage))) {
		redraw = 1;
		status_dirty = 1;
	} else {
		adjust_dirty(opage);
		status_dirty = 1;
	}

	if (redraw) {
		dirtyn = 1;
		dirty_all();
		cl_start = screen_height - 2;
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
		if (screen_height - 2 <= i || ebuf <= p)
			break;
		/* Normal characters */
		if (*p != '\n') {
			uint8_t s = con_size_x(*p, j);
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
	if (*dirtyp != 255) {
		if (!inpos)
			con_goto(i, j);
		con_clear_to_eol();
		*dirtyp = 255;
	}
	/* Now mark out the unused lines with ~ markers if needed */
	j = i + 1;
	while (++i < cl_start && i < screen_height - 2) {
		if (*dirtyp) {
			*dirtyp = 0;
			con_goto(i, 0);
			con_clear_to_eol();
		}
		dirtyp++;
	}
	cl_start = j;

	if (status_dirty) {
		con_goto(screen_height - 2, 0);
		con_reverse();
		con_puts("-- fleamacs: ");
		con_putsn(filetail, screen_width - 4);	
		con_putc(' ');
		while(screenx)	/* Will wrap at line end */
			con_putc('-');
		con_normal();
		status_dirty = 0;
	}
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

int main(int argc, char *argv[])
{
	uint8_t mem;
	uint8_t i;

	if (sizeof(void *) == 2) {
		buf = sbrk(0);
		ebuf = (char *)&mem - 768;
		if (ebuf < buf || brk(ebuf))
			oom();
	} else {
		buf = malloc(65535);
		if (buf == NULL)
			oom();
		ebuf = buf + 65535;
	}
	gap = buf;
	egap = ebuf;

	if (argc < 2)
		*filepath = 0;
	else {
		if (loadfile(*++argv))
			exit(2);
	}

	file_set_tail();

	if (con_init())
		return (3);

	con_goto(0,0);

	signal(SIGHUP, hupped);
	top();
	status_dirty = 1;
	display(1);

	while (!done) {
		display(0);
		i = 0;
		input = con_getch();
		if (keymode)
			input = toupper(input);
		if (!quoted)
			input |= keymode;

		if (status_up)
			status_wipe();

		if (quoted) {
			quoted = 0;
			insertch(input & 0xFF);
			continue;
		}

		while (table[i].key != 0 && input != table[i].key)
			++i;
		if (keymode && !table[i].func) { 
			dobeep();
			keymode = 0;
			continue;
		}
		keymode = 0;
		while(repeat--) {
			if (table[i].func)
				(*table[i].func) ();
			else if (input >= ' ' && input != 127)
				insertch(input);
			else {
				dobeep();
				break;
			}
			if (table[i].flags & NORPT)
				break;
		}
		if (!(table[i].flags & KEEPRPT))
			repeat = 1;
	}
	con_goto(screen_height - 1, 0);
	con_clear_to_eol();
	con_newline();
	con_flush();
	return (0);
}
