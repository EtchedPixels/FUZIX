/*
 *	Simple implementation of canonical style line input with redraw
 *	and timeout.
 *
 *	At the moment we only deal with the very simplest elements of the
 *	tty - no word delete, quoting etc just typing and deleting and
 *	going beep
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "linein.h"

static char *buffer, *bufstart, *bufpos, *bufend;

static void undraw(int len)
{
    int i;
    for (i = 0; i < len; i++)
        fputs("\010 \010", stdout);
    fflush(stdout);
}

static void redraw(void)
{
    if (bufpos != bufstart)
        fwrite(buffer, bufpos-bufstart, 1, stdout);
    fflush(stdout);
}

static uint8_t tty_do(char c)
{
    switch(c) {
    case 8:
    case 127:
        if (bufpos > bufstart)
            undraw(1);
        bufpos--;
        return 1;
    case '\n':
    case '\r':
        if (bufpos == bufstart)
            return 1;
        *bufpos++ = 0;
        putchar('\n');
        bufpos = bufstart;
        return 2;
    case 3:
        kill(getpid(), SIGINT);
        return 1;
    case ('\'' & 31):
        kill(getpid(), SIGQUIT);
        return 1;
    case 4:
        if (bufpos == bufstart)
            return 2;
        return 1;
    case 'U' - 64:
        undraw(bufpos - bufstart);
        bufpos = bufstart;
        return 1;
    case 'R' - 64:
        undraw(bufpos - buffer);
        redraw();
        return 1;
    case 9:
        c = ' ';
    default:
        if (c < 32 || c > 126)
            return 1;
        if (bufpos == bufend - 1) {
            putchar(7);
            fflush(stdout);
            return 1;
        }
        *bufpos++ = c;
        putchar(c);
        fflush(stdout);
        return 1;
    }
}
        
int tty_event(void)
{
    char c;
    int n;

    do {    
        n = read(0, &c, 1);
        if (n == -1 && errno != EAGAIN)
            return -1;
        if (n)
            n = tty_do(c);
    } while(n == 1);
    return n;
}

void tty_hide(void)
{
    undraw(bufpos - buffer);
}

void tty_show(void)
{
    redraw();
}

static struct termios term, save;
static uint8_t tty_saved;

int tty_width = 80;

int tty_begin(void)
{
    struct winsize sz;
    const char *p;
    int n;

    if (!isatty(0))
        return 0;
    if (tcgetattr(0, &save) == -1) {
        perror("tcgetattr");
        return -1;
    }
    tty_saved = 1;
    memcpy(&term, &save, sizeof(struct termios));
    term.c_lflag &= ~(ICANON|ECHO|ISIG);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 10;
    if (tcsetattr(0, TCSADRAIN, &term) == -1) {
        perror("tcsetattr");
        return -1;
    }
    if (ioctl(0, TIOCGWINSZ, &sz) == 0)
        tty_width = sz.ws_col;
    p = getenv("COLS");
    if (p) {
        n = atoi(p);
        if (n >= 20)
            tty_width = n;
    }
    return 0;
}

int tty_restore(void)
{
    return tcsetattr(0, TCSADRAIN, &save);
}

int tty_resume(void)
{
    return tcsetattr(0, TCSADRAIN, &term);
}

void tty_set_buffer(char *base, int promptlen, int totalsize)
{
    buffer = base;
    bufstart = base + promptlen;
    bufpos = bufstart;
    bufend = base + totalsize - 1;
}
