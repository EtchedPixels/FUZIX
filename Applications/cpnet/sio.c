/*************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix.
    This module implements low level serial port I/O routines.
  
    Copyright (C) 2004, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>

#include "main.h"
#include "sio.h"


/*----------------------------------------------------------------------*/

static int _fd = -1;

int sio_open(const char *sdev, int speed)
{
	struct termios ts;

	sio_close();

	_fd = open(sdev, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);

	if (_fd < 0) {
		perror(sdev);
		return -1;
	}

	if (tcgetattr(_fd, &ts) == -1) {
		error("failed to get tty settings\n");
		perror(sdev);
		close(_fd);
		_fd = -1;
		return -1;
	}

	cfmakeraw(&ts);
	ts.c_iflag = IGNBRK;
	ts.c_oflag = 0;
	ts.c_cflag = CS8 | CREAD | CLOCAL;
	ts.c_lflag = 0;
	ts.c_cc[VMIN] = 0; /* 1 */ ;
	ts.c_cc[VTIME] = 50;	/* one character at a time, 5 sec timeout */

	if (cfsetospeed(&ts, speed) == -1) {
		error("failed to set output speed\n");
		perror(sdev);
	}

	if (cfsetispeed(&ts, speed) == -1) {
		error("failed to set input speed\n");
		perror(sdev);
	}

	if (tcsetattr(_fd, TCSAFLUSH, &ts) == -1) {
		error("failed to set line attributes\n");
		perror(sdev);
	}

	/* Set the line back to blocking mode after setting CLOCAL */
	if (fcntl(_fd, F_SETFL, 0) < 0) {
		error("failed to set blocking mode\n");
		perror(sdev);
	}
#ifdef TIOCMBIC
	modemlines = TIOCM_RTS;
	if (ioctl(_fd, TIOCMBIC, &modemlines)) {
		/* can't clear RTS line, see ERRNO */
		error("failed to clear RTS line\n");
		perror(sdev);
	}
#endif
	return 0;
}

int sio_close(void)
{
	int retc;

	if (_fd == -1) {
		errno = EBADF;
		return -1;
	}

	retc = close(_fd);
	_fd = -1;

	return retc;
}

int sio_set_speed(int speed)
{
	struct termios ts;

	if (_fd < 0)
		return 0;

	if (tcgetattr(_fd, &ts) == -1) {
		perror(_sdev);
		return -1;
	}

	if (cfsetospeed(&ts, speed) == -1) {
		perror(_sdev);
		return -1;
	}

	if (cfsetispeed(&ts, speed) == -1) {
		perror(_sdev);
		return -1;
	}

	if (tcsetattr(_fd, TCSAFLUSH, &ts) == -1) {
		perror(_sdev);
		return -1;
	}

	return 0;
}

int sio_send(char *buf, int len)
{
	if (_fd < 0)
		return -1;
	if (len < 0)
		len = strlen(buf);
	if (write(_fd, buf, len) == len)
		return 0;
	return -1;
}

int sio_receive(char *buf, int len)
{
	int i, n;

	if (_fd < 0)
		return -1;
	for (i = 0; i < len; ++i) {
		n = read(_fd, &buf[i], 1);
		if (n == 0)
			break;	/* timeout! */
	}
	return i;
}
