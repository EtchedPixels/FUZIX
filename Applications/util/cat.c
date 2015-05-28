/*
 * tiny-cat.c - This file is part of the tiny-utils package for Linux & ELKS,
 * Copyright (C) 1995, 1996 Nat Friedman <ndf@linux.mit.edu>.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define STDIN_FILENO   0  /* fileno(stdin)  */
#define STDOUT_FILENO  1  /* fileno(stdout) */

#define BUFFER_SIZE    512

static char buff[BUFFER_SIZE];

/* The name of the file currently being displayed, "-" indicates stdin. */

char *filename;

int open_file(char *new_filename)
{
    int fd;

    filename = new_filename;

    if (*filename == '-' && *(filename + 1) == '\0')
	return (STDIN_FILENO);

    /*
     * If open() returns an error, the accepted behavior is for cat to
     * report the error and move on to the next file in the argument list.
     */
    if ((fd = open(filename, O_RDONLY)) < 0)
	perror(filename);

    return (fd);
}


/*
 * Output from the current fd until we reach EOF, and then return.
 */

int output_file(int fd)
{
    int bytes_read;

    while ((bytes_read = read(fd, buff, BUFFER_SIZE)) > 0)
	write(STDOUT_FILENO, buff, bytes_read);

    if (bytes_read < 0)
        perror(filename);

    return (1);
}

int main(int argc, char **argv)
{
    /* File descriptor for the input file */
    int curr_input_fd;
    int arg_num;

    if (argc == 1)
	arg_num = 0;
    else
	arg_num = 1;

    while (arg_num < argc) {
	if (argc == 1)
	    curr_input_fd = open_file("-");
	else
	    curr_input_fd = open_file(argv[arg_num]);

	if (curr_input_fd >= 0) {
	    output_file(curr_input_fd);
	    close(curr_input_fd);
	}
	arg_num++;
    }

    close(1);

    return (0);
}
