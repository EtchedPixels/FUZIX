/*
 *	A smaller saner readline
 *
 *	TODO:
 *	History save and load
 *	Test history overflow
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <readline/readline.h>

#undef CTRL			/* glibc defines this ?? */
#define CTRL(x)	((x) & 31)

#define WITH_HISTORY
#define MAX_HIST	8

static char *rl_end;
static char *rl_bufend;
static int rl_fd;
static char *rl_base;
static char *rl_cursor;
static uint8_t rl_inhist;

#ifdef WITH_HISTORY
static char *rl_hbuffer;
static char *rl_hend;
static char *rl_hbufend;

static char *hptr[MAX_HIST];
static size_t hlen[MAX_HIST];
static uint8_t histp = 0;
static uint8_t histend = 0;

static void history_set(void)
{
    register size_t s = hlen[histp];
    if (s > rl_bufend - rl_base)
        s = rl_bufend - rl_base;
    memcpy(rl_base, hptr[histp], s);
    rl_end = rl_base + s;
    rl_cursor = rl_end;
}

static void history_add(void)
{
    register uint_fast8_t i;
    size_t l;
    size_t s = rl_end - rl_base;
    
    if (rl_hbuffer == NULL)
        return;

    /* Remove the oldest entry until we fit */
    while(histend == MAX_HIST ||  rl_hend + s >= rl_hbufend) {
//        printf("Looping to clear for %d\n", s);
        l = hlen[0];
        rl_hend -= l;
        /* Shuffle the buffer */
        memmove(rl_hbuffer, rl_hbuffer + l, rl_hend - rl_hbuffer);
        /* Adjust the pointer/length table */
        memmove(hlen, hlen + 1, (MAX_HIST - 1) * sizeof(size_t));
        /* FIXME : rework as pointers */
        for (i = 1; i < MAX_HIST; i++)
            hptr[i-1] = hptr[i] - l;
        histend--;
        if (histp)
            histp--;
    }
    /* Add the new entry */
    memcpy(rl_hend, rl_base, s);
    hptr[histend] = rl_hend;
    hlen[histend++] = s;
    rl_hend += s;
}

static void history_droplast(void)
{
    histend--;
    rl_hend -= hlen[histend];
}

static void history_replace(void)
{
    history_droplast();
    history_add();
}
    

void rl_hinit(char *buffer, size_t len)
{
    rl_hbuffer = buffer;
    rl_hend = buffer;
    rl_hbufend = buffer + len;
}
#endif

static char buf[2] = "^";

static void quoted_putchar(char c)
{
    /* TODO - sort char < 32, and buffer */
    if ((uint8_t)c < 32) {
        buf[1] = c+'@';
        write(rl_fd, buf, 2);
    } else write(rl_fd, &c, 1);
}

static char dbuf[2] = "\b\b";

static void backspace(char c)
{
    write(rl_fd, dbuf, (c < 32) ? 2 : 1);
}

static void reprint_end(char c)
{
    /* Reprint cursor to len and then cursor back to the right place */
    register char *p = rl_cursor;

    /* Reprint from cursor to end */
    while(p < rl_end)
        quoted_putchar(*p++);
    if (c < 32)
        write(rl_fd, "  \b\b", 4);
    else
        write(rl_fd, " \b", 2);
    /* Go backwards to put the cursor back */
    while(p > rl_cursor)
        backspace(*--p);
}

static void reprint_start(void)
{
    register char *p = rl_base;
    /* Reprint up to cursor, leave cursor there */
    while(p <= rl_cursor)
        quoted_putchar(*p++);
}

static void insert(char c)
{
    if (rl_end < rl_bufend) {
        if (rl_cursor != rl_end) {
            memmove(rl_cursor + 1, rl_cursor, rl_end - rl_cursor);
            *rl_cursor++ = c;
            rl_end++;
            quoted_putchar(c);
            reprint_end(' ');
        } else {
            *rl_cursor++ = c;
            rl_end++;
            quoted_putchar(c);
        }
    }
}

static struct termios saved_term, term;
static uint8_t do_cleanup = 0;

static void exit_cleanup(void)
{
    if (do_cleanup == 1) {
        tcsetattr(0, TCSADRAIN, &saved_term);
        do_cleanup = 2;
    }
}

static void bspace(void)
{
    char c = *--rl_cursor;
    backspace(c);
    memmove(rl_cursor, rl_cursor + 1, rl_end - rl_cursor);
    --rl_end;
    reprint_end(c);
}

#ifdef WITH_HISTORY
static void history(int8_t dir)
{
    register char *p = rl_cursor;

    if (rl_hbuffer == NULL)
        return;
    if (!rl_inhist)
        histp = histend;
    if (histp + dir < 0 || histp + dir >= histend)
        return;
    while(p < rl_end)
        quoted_putchar(*p++);
    while (p != rl_base) {
        if (*--p < 32)
            write(rl_fd, "  \b\b", 4);
        else
            write(rl_fd, " \b", 2);
        backspace(*p);
    }
    write(rl_fd, "  \b\b", 4);
    if (histp + rl_inhist == histend) {
        if (rl_inhist)
            history_replace();
        else
            history_add();
    }
    rl_inhist = 1;
    histp += dir;
    history_set();
    rl_cursor = rl_base;
    while(rl_cursor < rl_end)
        quoted_putchar(*rl_cursor++);
}
#endif

int rl_edit_timeout(int fd, int ofd, const char *prompt,
                char *input, size_t len, uint8_t timeout, int (*timeout_fn)(void))
{
    uint8_t c;
    uint_fast8_t quote = 0;
    uint_fast8_t esc = 0;
    register uint_fast8_t r;

    rl_base = input;
    rl_end = input;
    rl_cursor = input;
    rl_bufend = input + len - 1;
    rl_fd = ofd;
    rl_inhist = 0;

    write(ofd, prompt, strlen(prompt));

    if (tcgetattr(0, &term) == 0) {
	memcpy(&saved_term, &term, sizeof(saved_term));
	if (!do_cleanup)
            atexit(exit_cleanup);
        do_cleanup = 1;
	term.c_lflag &= ~(ICANON|ECHO);
	/* We want 0, timeout or 1, 0 */
	term.c_cc[VMIN] = !timeout;
	term.c_cc[VTIME] = timeout;
	term.c_cc[VINTR] = 0;
	term.c_cc[VSUSP] = 0;
	term.c_cc[VSTOP] = 0;
	tcsetattr(0, TCSADRAIN, &term);
    }
    /* timeout support ?? */
    while(1) {
        r = read(fd, &c, 1);
        if (r != 1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                break;
            else {
                if (!timeout_fn())
                    continue;
                /* Timer asked for a redraw */
                c = CTRL('R');
            }
        } else {
            if (quote) {
                quote = 0;
                insert(c);
                continue;
            } else if (esc) {
                if (c == '[')
                    continue;
                esc = 0;
                if (c >= 'A' && c <= 'N')
                    c = CTRL("PNFB   A     E"[c - 'A']);
            }
            else if (c == '\n')
                break;
        }
        switch(c) {
            case '\033':
                esc = 1;
                break;
            case '\r':
                break;
            case CTRL('C'):
                write(2, "^C\n", 3);
                exit_cleanup();
                return -2;
            case CTRL('D'):
                /* Fake EOF check */
                r = 0;
                goto out;
            case '\b':
            case 127:
                if (rl_cursor != rl_base)
                    bspace();
                break;
            case CTRL('U'):
            case CTRL('X'):
                while (rl_cursor != rl_base)
                    bspace();
                break;
            case CTRL('V'):
                quote = 1;
                /* Quote */
                break;
            case CTRL('F'):
                if (rl_cursor < rl_end)
                    quoted_putchar(*rl_cursor++);
                break;
            case CTRL('B'):
                if (rl_cursor > rl_base) {
                    backspace(*--rl_cursor);
                }
                break;
            case CTRL('E'):
                while(rl_cursor < rl_end)
                    quoted_putchar(*rl_cursor++);
                break;
            case CTRL('A'):
                while(rl_cursor != rl_base ) {
                    backspace(*--rl_cursor);
                }
                break;
#ifdef WITH_HISTORY                
            case CTRL('P'):
                history(-1);
                break;
            case CTRL('N'):
                history(1);
                break;
#endif                
            case CTRL('R'):
                write(ofd, "\n", 1);
                write(ofd, prompt, strlen(prompt));
                reprint_start();
                reprint_end(' ');
                break;
            default:
                if (c > 31)
                    insert(c);
                break;
        }
    }
out:
#ifdef WITH_HISTORY
    if (rl_inhist) {
        if (rl_end != rl_base)
            history_replace();
        else
            history_droplast();
    }
    else if (rl_end != rl_base)
        history_add();
#endif
    write(ofd, "\n", 1);
    exit_cleanup();
    if (rl_end == rl_base && r == 0)
        return -1;		/* EOF */
    return rl_end - rl_base;
}

int rl_edit(int fd, int ofd, const char *prompt, char *input, size_t len)
{
    return rl_edit_timeout(fd, ofd, prompt, input, len, 0, NULL);
}

#ifdef TEST
#include <stdio.h>
char historybuf[384];

int main(int argc, char *argv[])
{
    char buf[256];
    int l;
//    rl_hinit(historybuf, 384);
    while(1) {
        l = rl_edit(0, 1, "> ", buf, 256);
        if (l > 0) {
            buf[l] = 0;
            printf("\"%s\"\n", buf);
        }
        if (l == -1)
            break;
    }
}
#endif
