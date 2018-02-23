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

#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <string.h>

#ifndef BUF
#define BUF        16384
#endif /* BUF */

#ifndef HUP
#define HUP        "ae.hup"
#endif /* HUP */

typedef struct keytable_t {
        int key;
        int flags;
#define NORPT	1
#define KEEPRPT	2
        int (*func)(void);
} keytable_t;

int done;
int row, col;
int indexp, page, epage;
int input;
int repeat;
char buf[BUF];
char *ebuf;
char *gap = buf;
char *egap;
char *filename;
keytable_t *table;

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
 *	Add some minimal : commands (:w notably and :n)
 *	Yank/paste
 *	Remove stdio and curses use
 *	ZZ
 *	Use uint8, uint when we can for speed
 *	Use memmove not loops
 *	Macros out of the command blocks we have - may need them all to return
 *	an error status
 *	: blank lines vi style
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

void display(void);
void movegap(void);
int insertch(char);
int fleft(char);
int fright(char);

int backsp(void);
int bottom(void);
int delete(void);
int delete_line(void);
int down(void);
int file(void);
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
int redraw(void);
int right(void);
int replace(void);
int quit(void);
int flip(void);
int top(void);
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

#undef CTRL
#define CTRL(x)                ((x) & 0x1f)

keytable_t modeless[] = {
        { KEY_LEFT, 0, left },
        { KEY_RIGHT, 0, right },
        { KEY_DOWN, 0, down },
        { KEY_UP, 0, up },
        { CTRL('w'), 0, wleft },
        { CTRL('e'), 0, wright },
        { CTRL('n'), 0, pgdown },
        { CTRL('p'), 0, pgup },
        { CTRL('a'), NORPT, lnbegin },
        { CTRL('d'), NORPT, lnend },
        { CTRL('t'), NORPT, top },
        { CTRL('b'), NORPT, bottom },
        { KEY_BACKSPACE, 0, backsp },
        { '\b', 0, backsp },
        { KEY_DC, 0, delete },
        { CTRL('f'), NORPT, file },
        { CTRL('r'), NORPT, redraw },
        { CTRL('\\'), NORPT, quit },
        { CTRL('z'), NORPT, flip },
        { 0, 0, insert }
};

/*
 *	Basic vi commands missing (note not all below exactly match vi yet
 *	in their behaviour - much testing is needed
 *
 *	CTRL-D/CTRL-U	half screen scrolls
 *	CTRL-F/CTRL-B	implemented but not exactly as vi
 *	nG		goto line n
 *	W and S		word skip with punctuation
 *	[] and ()	setence and paragraph skip
 *	H M L		top/niddle/bottom of screen
 *	R		replace mode
 *	t		swap two characters
 *	cw/cc/c.	change word, change line, change one char (+c$ etc)
 *	C		change to end of line
 *	s		substitute
 *	u		undo
 *	dw,dd,de,d$	delete word/space, delete line, delete word, delete
 *			to eol (and d. I think delete char) d^ delete to
 *			start of line
 *	D		synonum for d$ (aka c$ = C)
 *	f/Fc		move forward and back to char c
 *	%		move to associated bracket pair
 *	.		repeat last text changing command
 *
 *	All : functionality
 *	All yank/put functionality
 *	All regexps /?nN
 *
 *	Try to write ops as far as possible in terms of each other and a few
 *	non-command 'ops. The goal is to make a lot of this macrocode for size.
 */

keytable_t modual[] = {
        { KEY_LEFT, 0, left },
        { KEY_RIGHT, 0, right },
        { KEY_DOWN, 0, down },
        { KEY_UP, 0, up },
        {  27, NORPT, noop },
        { 'h', 0, left },
        { '\b', 0, left },
        { 'j', 0, down },
        { '\n', 0, down },
        { 'k', 0, up },
        { 'l', 0, right },
        { ' ', 0, right },
        { 'b', 0, wleft },
        { 'e', 0, eword },
        { CTRL('F'), 0, pgdown },	/* Need D for half screen too */
        { CTRL('B'), 0, pgup },
        { CTRL('U'), 0, pgup_half },
        { 'w', 0, wright },
        { '^', NORPT, lnbegin },
        { '$', NORPT, lnend },
        { 't', NORPT, top },		/* Should be 0G */
        { 'b', NORPT, bottom },		/* Should be G  b is begin of word */
        { 'i', NORPT, insert_mode },
        { 'I', NORPT, insert_before },
        { 'J', 0, join },
        { 'x', 0, delete },
        { 'X', 0, delete_left },
        { 'o', 0, open_after },
        { 'O', 0, open_before },
        { 'W', NORPT, file },
        { 'R', NORPT, redraw },
        { 'Q', NORPT, quit },
        { 'Z', NORPT, flip },
        { 'd', 0, delete_line },	/* Should be dd */
        { 'a', NORPT, append_mode },
        { 'A', NORPT, append_end },
        { 'r', 0, replace },
        { 'F', NORPT, findleft },
        { 'f', NORPT, findright },
        { '0', KEEPRPT, digit },
        { '1', KEEPRPT, digit },
        { '2', KEEPRPT, digit },
        { '3', KEEPRPT, digit },
        { '4', KEEPRPT, digit },
        { '5', KEEPRPT, digit },
        { '6', KEEPRPT, digit },
        { '7', KEEPRPT, digit },
        { '8', KEEPRPT, digit },
        { '9', KEEPRPT, digit },
        { 0, KEEPRPT, noop }
};


char *ptr(int offset)
{
        if (offset < 0)
                return (buf);
        return (buf+offset + (buf+offset < gap ? 0 : egap-gap));
}

int pos(char *pointer)
{
        return (pointer-buf - (pointer < egap ? 0 : egap-gap));
}

int top(void)
{
        indexp = 0;
        return 0;
}

int bottom(void)
{
        epage = indexp = pos(ebuf);
        return 0;
}

int quit(void)
{
        done = 1;
        return 0;
}

int redraw(void)
{
        clear();
        display();
        return 0;
}

int digit(void)
{
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
        while (buf < (p = ptr(--offset)) && *p != '\n')
                ;
        return (buf < p ? ++offset : 0);
}

int nextline(int offset)
{
        char *p;
        while ((p = ptr(offset++)) < ebuf && *p != '\n')        
                ;
        return (p < ebuf ? offset : pos(ebuf));
}

int adjust(int offset, int column)
{
        char *p;
        int i = 0;
        while ((p = ptr(offset)) < ebuf && *p != '\n' && i < column) {
                i += *p == '\t' ? 8-(i&7) : 1;
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
        indexp = adjust(prevline(prevline(indexp)-1), col);
        return 0;	/* FIXME */
}

int down(void)
{
        indexp = adjust(nextline(indexp), col);
        return 0;	/* FIXME */
}

int lnbegin(void)
{
        indexp = prevline(indexp);
        return 0;
}

int lnend(void)
{
        indexp = nextline(indexp);
        return left();		/* FIXME? if on end of last lie already ? */
}
/* We want to the top of the next page: in theory I believe we shouldn't
   move at all unless we can move this far. Need to save pointers and test ? */
int pgdown(void)
{
        /* Go to bottom of our page */
        while(row < LINES) {
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
        int i = LINES;
        /* Not quite right but will do for now */
        if (page == 0)
                return 1;
        /* Go to the bottom of the page */
        while(row < LINES) {
                row++;
                down();
        }
        /* Now go up a page, moving the page marker as we do
           FIXME: just do the difference!!! */
        while (0 < --i) {
                page = prevline(page-1);
                up();
        }
        return 0;
}

/* TODO FIXME */
int pgup_half(void)
{
        int i = LINES/2;
        while (0 < --i) {
                page = prevline(page-1);
                up();
        }
        return 0;
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
        int c = getch();
        if (c < 0 || c > 255)
                return 1;
        return fleft(c);
}

int findright(void)
{
        int c = getch();
        if (c < 0 || c > 255)
                return 1;
        return fright(c);
}

/* Do we need a filter on this and insert_mode ? */
int insertch(char ch)
{
        movegap();
        if (gap < egap) {
                *gap++ = ch == '\r' ? '\n' : ch;
                indexp = pos(egap);
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
        display();
        return insert_mode();
}

/* Need to do a replace mode version */
int insert_mode(void)
{
        int ch;
        movegap();
        while ((ch = getch()) != 27) {
                if (ch == 0)
                        continue;
                if (ch == '\f')
                        break;
                if (ch == '\b') {
                        if (buf < gap)
                                --gap;
                } else if (gap < egap) {
                        *gap++ = ch == '\r' ? '\n' : ch;
                }
                indexp = pos(egap);
                display();
        }
        return 0;
}

int append_mode(void)
{
        if (!right()) {
                display();
                return insert_mode();
        }
        return 0;
}

int append_end(void)
{
        lnend();
        return append_mode();
}

int replace(void)
{
        int c = getch();
        /* FIXME: saner filter ? */
        if (c < 0 || c > 255)
                return 1;
        if (!delete())
                return insertch(c);
        return 0;
}

int delete_line(void)
{
        lnbegin();
        while(egap < ebuf - 1 && egap[1] != '\n')
                indexp = pos(++egap);
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

int file(void)
{
        if (!save(filename))
                save(HUP);
        return 0;
}

int save(char *fn)
{
        FILE *fp;
        int i, ok;
        size_t length;
        fp = fopen(fn, "w");
        if ((ok = fp != NULL)) {
                i = indexp;
                indexp = 0;
                movegap();
                length = (size_t) (ebuf-egap);
                ok = fwrite(egap, sizeof (char), length, fp) == length;
                (void) fclose(fp);
                indexp = i;
        }
        return (ok);
}

int flip(void)
{
        table = table == modual ? modeless : modual;
        return 0;
}

int noop(void)
{
        return 0;
}

void display(void)
{
        char *p;
        int i, j;
        if (indexp < page)
                page = prevline(indexp);
        if (epage <= indexp) {
                page = nextline(indexp);
                i = page == pos(ebuf) ? LINES-2 : LINES;
                while (0 < i--)
                        page = prevline(page-1);
        }
        move(0, 0);
        i = j = 0;
        epage = page;
        while (1) {
                if (indexp == epage) {
                        row = i;
                        col = j;
                }
                p = ptr(epage);
                if (LINES <= i || ebuf <= p)
                        break;
                if (*p != '\r') {
                        addch(*p);
                        j += *p == '\t' ? 8-(j&7) : 1;
                }
                if (*p == '\n' || COLS <= j) {
                        ++i;
                        j = 0;
                }
                ++epage;
        }
        clrtobot();
        while(++i < LINES)
                mvaddstr(i, 0, "~");
        move(row, col);
        refresh();
}

int main(int argc, char *argv[])
{
        FILE *fp;
        char *p = *argv;
        int i = (int) strlen(p);
        egap = ebuf = buf + BUF;
        if (argc < 2)
                return (2);
        /* Find basename. */
        while (0 <= i && p[i] != '\\' && p[i] != '/')
                --i;
        p += i+1;
        if (strncmp(p, "vi", 2) == 0)
                table = modual;
        else if (strncmp(p, "ev", 2) == 0 || strncmp(p, "ev", 2) == 0)
                table = modeless;
        else
                return (2);
        if (initscr() == NULL)
                return (3);
        raw();
        noecho();
        idlok(stdscr, 1);
        keypad(stdscr, 1);
        fp = fopen(filename = *++argv, "r");
        if (fp != NULL) {
                gap += fread(buf, sizeof (char), (size_t) BUF, fp);
                fclose(fp);
        }
        top();
        while (!done) {
                display();
                i = 0;
                input = getch();
                while (table[i].key != 0 && input != table[i].key)
                        ++i;
                if (!repeat || (table[i].flags & NORPT))
                        (*table[i].func)();
                else while(repeat--)
                        (*table[i].func)();
                if (!(table[i].flags & KEEPRPT))
                        repeat = 0;
        }
        endwin();
        return (0);
}
