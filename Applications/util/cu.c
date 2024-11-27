/* cu.c - Simple serial console.
 *
 * Copyright (C) 2024 Yaroslav Veremenko, All rights reserved.
 *
 * This file is part of FUZIX Operating System.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <getopt.h>

static struct termios termsave[2];
static struct termios termcur[2];
static int remotefd = -1;
static pid_t child;

static int baud[] = {
    50,     /* B50 */
    75,     /* B75 */
    110,    /* B110 */
    134,    /* B134 */
    150,    /* B150 */
    300,    /* B300 */
    600,    /* B600 */
    1200,   /* B1200 */
    2400,   /* B2400 */
    4800,   /* B4800 */
    9600,   /* B9600 */
    19200,  /* B19200 */
    38400,  /* B38400 */
    57600,  /* B57600 */
    115200, /* B115200 */
};

static speed_t speed[] = {
    B50,
    B75,
    B110,
    B134,
    B150,
    B300,
    B600,
    B1200,
    B2400,
    B4800,
    B9600,
    B19200,
    B38400,
    B57600,
    B115200,
};

#ifndef uint_fast8_t
#define uint_fast8_t char
#endif

#define TTY 0
#define REMOTE 1

static void term_raw(int fd, int type)
{
    tcgetattr(fd, &termsave[type]);
    memcpy(&termcur[type], &termsave[type], sizeof(struct termios));
    cfmakeraw(&termcur[type]);
    termcur[type].c_cc[VMIN] = 1;
    termcur[type].c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &termcur[type]);
}

static void restore()
{
    tcsetattr(0, TCSAFLUSH, &termsave[TTY]);
    if (remotefd >= 0)
    {
        tcsetattr(remotefd, TCSAFLUSH, &termsave[REMOTE]);
        close(remotefd);
    }
}

static void quit()
{
    kill(child, SIGKILL);
    wait((int *)NULL);
    restore();
}

static int parsespeed(char *str, speed_t *s)
{
    register int i;
    register int b = atoi(str);
    for (i = 0; i < sizeof(baud) / sizeof(baud[0]); i++)
    {
        if (baud[i] == b)
        {
            *s = speed[i];
            return 1;
        }
    }
    return 0;
}

static void usage()
{
    fputs("Usage: cu -l LINE [-s SPEED] [-E ESCCHAR]\n",
          stderr);
}

int main(int argc, char *argv[])
{
    speed_t speedval = 0;
    char escchar = '~';
    int opt;
    char *remotename = NULL;
    char escStr[3];

    if (isatty(0) != 1)
    {
        fputs("fd 0 is not tty\n", stderr);
        exit(1);
    }

    while ((opt = getopt(argc, argv, "s:l:E:")) != -1)
    {
        switch (opt)
        {
        case 's':
            if (!parsespeed(optarg, &speedval))
            {
                fprintf(stderr, "invalid speed: %s\n", optarg);
                exit(1);
                return 0;
            }
            break;
        case 'l':
            remotename = optarg;
            break;
        case 'E':
            /* if arg starts with ^, then ^@=0x00 and ^Z=0x1A
                otherwise escape char is used verbatim */
            if (optarg[0] == '^' && (optarg[1] < 'A' || optarg[1] > 'Z') || optarg[1])
            {
                fprintf(stderr, "invalid char\n");
                exit(1);
                return 0;
            }
            escchar = optarg[0];
            if (optarg[0] == '^')
            {
                escchar -= '@';
            }
            break;
        default:
            usage();
            exit(1);
            break;
        }
    }

    if (remotename == NULL)
    {
        usage();
        exit(1);
    }

    remotefd = open(remotename, O_RDWR | O_NOCTTY);

    if (remotefd < 0)
    {
        perror(remotename);
        exit(1);
    }

    term_raw(0, TTY);
    term_raw(remotefd, REMOTE);

    if (speedval > 0)
    {
        if (cfsetospeed(&termcur[REMOTE], (speed_t)speedval) < 0 ||
            tcsetattr(remotefd, TCSAFLUSH, &termcur[REMOTE]) < 0)
        {
            restore();
            perror("speed");
            exit(1);
        }
    }

    child = fork();
    if (child < 0)
    {
        restore();
        perror("fork");
        exit(1);
    }
    if (child == 0)
    {
        uint_fast8_t r;
        while (read(remotefd, &r, 1) == 1)
        {
            write(0, &r, 1);
        }
        fputs("Disconnected\r\n", stderr);
    }
    else
    {
        uint_fast8_t is_esc = 0;
        uint_fast8_t is_nl = 1;
        uint_fast8_t w;
        atexit(restore);
        if (escchar <= 0x1A)
        {
            escStr[0] = '^';
            escStr[1] = escchar + '@';
            escStr[2] = '\0';
        }
        else
        {
            escStr[0] = escchar;
            escStr[1] = '\0';
        }

        fprintf(stderr, "Connected. Escape is %s\r\n", escStr);
        while (read(0, &w, 1) == 1)
        {
            if (is_nl && !is_esc && w == escchar)
            {
                is_esc = 1;
                continue;
            }
            if (is_esc && w == '.')
            {
                break;
            }
            is_nl = w == '\r' || w == '\n';
            is_esc = 0;
            write(remotefd, &w, 1);
        }
        quit();
    }
}