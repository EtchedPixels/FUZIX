/*************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix.

    This module implements the serial port communicarion protocol
    described in Appendix E of the CP/NET documentation.
  
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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>

#include "main.h"
#include "netio.h"
#include "sio.h"


/*----------------------------------------------------------------------*/

void wait_for_packet(void)
{
	unsigned char buf[2];

	/* wait for ENQ byte... */
	while (1) {
		while (sio_receive((char *)buf, 1) < 1) {
		}
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[0]);
#endif
		if (buf[0] == ENQ)
			break;
	}

	/* send ACK back */
	buf[0] = ACK;
	sio_send((char *)buf, 1);
#ifdef DEBUG
	if (_debug & DEBUG_PACKET)
		printf("\t<< %02X\n", buf[0]);
#endif
}

int get_packet(unsigned char *data, int *len, int *fnc, int *sid)
{
	int i, n, did, siz;
	unsigned char cks;
	static unsigned char buf[260];

	/* receive header */
	n = sio_receive((char *)buf, 7);
	cks = 0;
	for (i = 0; i < n; ++i) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
#endif
		cks += buf[i];
	}
	if (buf[0] != SOH || n != 7) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Bad packet\n\n");
#endif
		return -1;
	}
	if (cks != 0) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Checksum error\n\n");
#endif
		return -1;
	}
	did = buf[2];
	*sid = buf[3];
	*fnc = buf[4];
	siz = buf[5];
	*len = siz + 1;
	if (did != _netID) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Not for us...\n\n");
#endif
		return -1;
	}

	/* send ACK */
	buf[0] = ACK;
	sio_send((char *)buf, 1);
	if (_debug & DEBUG_PACKET)
		printf("\t<< %02X\n", buf[0]);

	/* receive data part */
	n = sio_receive((char *)buf, siz + 2);
	cks = 0;
	for (i = 0; i < n; ++i) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
#endif
		cks += buf[i];
	}
	if (buf[0] != STX) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Bad packet\n\n");
#endif
		return -1;
	}
	if (n != siz + 2) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Bad length\n\n");
#endif
		return -1;
	}
	for (i = 0; i < siz + 1; ++i) {
		data[i] = buf[i + 1];
	}

	/* receive checksum field */
	n = sio_receive((char *)buf, 2);
	for (i = 0; i < n; ++i) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
#endif
		cks += buf[i];
	}
	if (buf[0] != ETX || n != 2) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Bad packet\n\n");
#endif
		return -1;
	}
	if (cks != 0) {
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("Checksum error\n\n");
#endif
		return -1;
	}

	/* receive trailer */
	n = sio_receive((char *)buf, 1);
	for (i = 0; i < n; ++i) {
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
	}
	if (buf[0] != EOT || n != 1) {
		if (_debug & DEBUG_PACKET)
			printf("Bad packet\n\n");
		return -1;
	}

	/* send ACK */
	buf[0] = ACK;
	sio_send((char *)buf, 1);
	if (_debug & DEBUG_PACKET)
		printf("\t<< %02X\n", buf[0]);

	return 0;
}

int send_packet(int to, int fnc, unsigned char *data, int len)
{
	int i, n;
	unsigned char cks;
	static unsigned char buf[260];
#ifdef DEBUG
	if (_debug & DEBUG_DATA) {
		printf("Replying\n");
		dump_data(data, len);
		printf("\n");
	}
#endif
	if (len < 1 || len > 256) {
		error("internal: oversized packet\n");
		return -1;
	}

	buf[0] = ENQ;
	sio_send((char *)buf, 1);
#ifdef DEBUG
	if (_debug & DEBUG_PACKET)
		printf("\t<< %02X\n", buf[0]);
#endif
	/* wait for ACK */
	n = sio_receive((char *)buf, 1);
#ifdef DEBUG
	for (i = 0; i < n; ++i) {
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
	}
#endif
	buf[0] = SOH;
	buf[1] = 1;		/* FMT */
	buf[2] = to;		/* DID */
	buf[3] = _netID;	/* SID */
	buf[4] = fnc;		/* FNC */
	buf[5] = len - 1;	/* SIZ */
	cks = 0;
	for (i = 0; i < 6; ++i) {
		cks += buf[i];
		if (_debug & DEBUG_PACKET)
			printf("\t<< %02X\n", buf[i]);
	}
	buf[6] = -cks;		/* HCS */
	if (_debug & DEBUG_PACKET)
		printf("\t<< %02X\n", buf[6]);
	sio_send((char *)buf, 7);
	/* wait for ACK */
	sio_receive((char *)buf, 1);
#ifdef DEBUG
	for (i = 0; i < n; ++i) {
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
	}
#endif
	buf[0] = STX;
	for (i = 0; i < len; ++i) {
		buf[i + 1] = data[i];
	}

	buf[len + 1] = ETX;
	cks = 0;
	for (i = 0; i < len + 2; ++i) {
		cks += buf[i];
#ifdef DEBUG
		if (_debug & DEBUG_PACKET)
			printf("\t<< %02X\n", buf[i]);
#endif
	}
	buf[len + 2] = -cks;
	buf[len + 3] = EOT;
#ifdef DEBUG
	if (_debug & DEBUG_PACKET) {
		printf("\t<< %02X\n", buf[len + 2]);
		printf("\t<< %02X\n", buf[len + 3]);
	}
#endif
	sio_send((char *)buf, len + 4);
	/* wait for ACK */
	n = sio_receive((char *)buf, 1);
#ifdef DEBUG
	for (i = 0; i < n; ++i) {
		if (_debug & DEBUG_PACKET)
			printf(">> %02X\n", buf[i]);
	}

	if (n < 1) {
		if (_debug & DEBUG_PACKET)
			printf("No ACK?\n\n");
	}
#endif
	return 0;
}

int send_ok(int to, int fnc)
{
	unsigned char uc = 0;
	return send_packet(to, fnc, &uc, 1);
}

int send_error(int to, int fnc)
{
	return send_packet(to, fnc, (unsigned char *)"\xFF\x0C", 2);
}
