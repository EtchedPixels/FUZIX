/**************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix.
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

**************************************************************************/

#ifndef __netio_h
#define __netio_h

#define SOH   1
#define STX   2
#define ETX   3
#define EOT   4
#define ENQ   5
#define ACK   6


void wait_for_packet(void);
int get_packet(unsigned char *data, int *len, int *fnc, int *sid);
int send_packet(int to, int fnc, unsigned char *data, int len);
int send_ok(int to, int fnc);
int send_error(int to, int fnc);


#endif  /* __netio_h */
