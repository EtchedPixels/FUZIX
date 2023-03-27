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

#ifndef __main_h
#define __main_h

#define VERSION "0.8"

#define DEBUG_PACKET  0x01
#define DEBUG_DATA    0x02
#define DEBUG_MISC    0x04

void usage(char *pname);
void dump_data(unsigned char *buf, int len);
int goto_drive(int drive);
/* int lst_output(int num, char *buf, int len); */
int set_speed(unsigned long baud);
unsigned long get_baud(speed_t speed);
void error(const char *p);

extern int _netID;
extern int _debug;
extern int _logged_in;
extern char _passwd[8];
extern const char *_sdev;
extern const char *disk_to_dir[16];
extern uint8_t allocv[256];
#endif  /* __main_h */
