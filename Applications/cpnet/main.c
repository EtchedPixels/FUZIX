/*************************************************************************

    CP/NET 1.1/1.2 server emulator for Unix systems.

    This program is used to communicate with a single CP/M requester
    connected to a PC serial port using the RS232 protocol described
    in Appendix E of the CP/NET documentation.
  
    The program emulates either version 1.1 or 1.2 of CP/NET, and
    therefore must be used with a requester running the same CP/NET
    version.

    CP/NET versions 1.0, 1.1 and 1.2 are NOT compatible with each other.
    Although they may use the same physical-layer protocol (for a serial
    line, for example), they differ at the network-layer level.

    Copyright (C) 2005, Hector Peraza.

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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>

#include "main.h"
#include "cpnet12.h"
#include "netio.h"
#include "sio.h"
#include "cpmutl.h"


/*----------------------------------------------------------------------*/

int _netID = 0;			/* our server ID */
int _debug = 0;			/* debug mask */

int _logged_in = 0;
char _passwd[8];

const char *_sdev;
int _speed;

/* CP/M disk number to Unix directory mappings */
const char *disk_to_dir[16];

#ifdef WITH_LPT			/* TODO */
/* LST to Unix device mappings */
struct lstmap {
	char *fname;
	FILE *f;
} lst_to_dev[8];
#endif

#ifdef DEBUG

/* CP/NET function names, for debugging */
const char *fn_name[] = {
	/* 00 */ "system reset",
	/* 01 */ "console input",
	/* 02 */ "console output",
	/* 03 */ "raw console input",
	/* 04 */ "raw console output",
	/* 05 */ "list output",
	/* 06 */ "direct console I/O",
	/* 07 */ "get I/O byte",
	/* 08 */ "set I/O byte",
	/* 09 */ "print string",
	/* 10 */ "read console buffer",
	/* 11 */ "get console status",
	/* 12 */ "get version number",
	/* 13 */ "reset disk system",
	/* 14 */ "select disk",
	/* 15 */ "open file",
	/* 16 */ "close file",
	/* 17 */ "search first",
	/* 18 */ "search next",
	/* 19 */ "delete file",
	/* 20 */ "read sequential",
	/* 21 */ "write sequential",
	/* 22 */ "create file",
	/* 23 */ "rename file",
	/* 24 */ "get login vector",
	/* 25 */ "get current disk",
	/* 26 */ "set DMA address",
	/* 27 */ "get allocation vector",
	/* 28 */ "write protect disk",
	/* 29 */ "get R/O vector",
	/* 30 */ "set file attributes",
	/* 31 */ "get DPB",
	/* 32 */ "get/set user code",
	/* 33 */ "read random",
	/* 34 */ "write random",
	/* 35 */ "compute file size",
	/* 36 */ "set random record",
	/* 37 */ "reset drive",
	/* 38 */ "access drive",
	/* 39 */ "free drive",
	/* 40 */ "write random with zero fill",

	/* 64 */ "login",
	/* 65 */ "logoff",
	/* 66 */ "send message on network",
	/* 67 */ "receive message on network",
	/* 68 */ "get network status",
	/* 69 */ "get config table address"
};

#endif

/*----------------------------------------------------------------------*/

static const char *appname;

static void writes(int fd, const char *p)
{
	write(fd, p, strlen(p));
}

void error(const char *p)
{
	writes(2, appname);
	write(2, ": ", 2);
	writes(2, p);
}

int main(int argc, char *argv[])
{
	int opt;

	appname = argv[0];

	/* initializations */

	_sdev = "/dev/ttyS0";
	_speed = B38400;
	strncpy(_passwd, "PASSWORD", 8);

	while ((opt = getopt(argc, argv, "t:d:p:s:A:B:C;D:E:F:G:H:I:J:K:L:M:N:O:P:")) != -1) {
		if (opt >= 'A' && opt <= 'P') {
			disk_to_dir[opt - 'A'] = optarg;
			if (*optarg != '/') {
				error("paths must be absolute.\n");
				exit(1);
			}
		} else switch (opt) {
		case 't':
			_debug = atoi(optarg);
			break;
		case 'd':
			_sdev = optarg;
			break;
		case 's':
			set_speed(atol(optarg));
			break;
		case 'p':
			strncpy(_passwd, optarg, 8);
			break;
		case 'n':
			_netID = atoi(optarg);
			break;
		default:
			usage(argv[0]);
		}
	}
	if (optind != argc)
		usage(argv[0]);

	if (!disk_to_dir[0]) {
		error("drive A mapping required.\n");
		exit(1);
	}

#ifdef DEBUG
	if (_debug & DEBUG_MISC) {
		for (i = 0; i < 16; ++i) {
			if (disk_to_dir[i])
				printf("%c: = %s\n", i + 'A', disk_to_dir[i]);
		}
	}
#endif

	/* TODO */
#ifdef WITH_LPT
	for (i = 0; i < 8; ++i) {
		if (lst_to_dev[i].fname) {
			if (_debug & DEBUG_MISC)
				printf("LST%d: = %s\n", i, lst_to_dev[i].fname);
			if (strcmp(lst_to_dev[i].fname, "-") == 0) {
				lst_to_dev[i].f = stdout;
			} else if (strcmp(lst_to_dev[i].fname, "--") == 0) {
				lst_to_dev[i].f = stderr;
			} else {
				lst_to_dev[i].f = fopen(lst_to_dev[i].fname, "a");
				if (!lst_to_dev[i].f) 
					fprintf(stderr, "%s: could not open %s for lst%d output\n", argv[0], lst_to_dev[i].fname, i);
				}
			}
		}
	}
#endif
	if (sio_open(_sdev, _speed) != 0) {
		error("could not open serial\n");
		return 1;
	}

#ifdef DEBUG
	if (_debug & DEBUG_MISC) {
		printf("%s open at %ld baud\n", _sdev, get_baud(_speed));
		printf("server id is %d\n", _netID);
		printf("network password is %.8s\n", _passwd);
		printf("entering server loop...\n");
	}
#endif
	/* main loop - inside cpnet_11() and cpnet_12() routines */

	cpnet_12();

	/* right now we never get here */
	return 0;
}

void usage(char *pname)
{
	writes(1, "usage: ");
	writes(1, pname);
	writes(1, " [options]\n");
	writes(1, " This program is free software; you can redistribute it and/or\n");
	writes(1, " modify it under the terms of the GNU General Public License.\n");
}

#ifdef DEBUG
void dump_data(unsigned char *buf, int len)
{
	int i, addr;

	addr = 0;
	while (len > 0) {
		printf("%04X: ", addr);
		for (i = 0; i < 16; ++i) {
			if (len > i)
				printf("%02X ", buf[addr + i]);
			else
				printf("   ");
		}
		for (i = 0; i < 16; ++i) {
			if (len > i)
				printf("%c", (buf[addr + i] >= 32 && buf[addr + i] < 127) ? buf[addr + i] : '.');
			else
				printf(" ");
		}
		addr += 16;
		len -= 16;
		printf("\n");
	}
}
#endif

/*----------------------------------------------------------------------*/

int goto_drive(int drive)
{
	if (disk_to_dir[drive])
		return chdir(disk_to_dir[drive]);
	else
		return -1;
}

#ifdef WITH_LPT	/* TODO */
int lst_output(int num, char *buf, int len)
{
	int i;

	if (num < 0 || num >= 8)
		return -1;
	if (!lst_to_dev[num].f)
		return -1;

	for (i = 0; i < len; ++i) {
		if (buf[i] == 0x1a)
			break;
		fputc(buf[i], lst_to_dev[num].f);
	}
	fflush(lst_to_dev[num].f);

	return 0;
}
#endif

int set_speed(unsigned long baud)
{
	switch (baud) {
	case 300:
		_speed = B300;
		break;
	case 600:
		_speed = B600;
		break;
	case 1200:
		_speed = B1200;
		break;
	case 2400:
		_speed = B2400;
		break;
	case 4800:
		_speed = B4800;
		break;
	case 9600:
		_speed = B9600;
		break;
	case 19200:
		_speed = B19200;
		break;
	case 38400:
		_speed = B38400;
		break;
	case 57600:
		_speed = B57600;
		break;
	case 115200:
		_speed = B115200;
		break;
	default:
		_speed = B38400;
		return -1;
	}
	return 0;
}

unsigned long get_baud(speed_t speed)
{
	switch (speed) {
	case B300:
		return 300;
	case B600:
		return 600;
	case B1200:
		return 1200;
	case B2400:
		return 2400;
	case B4800:
		return 4800;
	case B9600:
		return 9600;
	case B19200:
		return 19200;
	case B38400:
		return 38400;
	case B57600:
		return 57600;
	case B115200:
		return 115200;
	}
	return 0;
}
