/**************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix.
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

**************************************************************************/

#ifndef __sio_h
#define __sio_h

int sio_open(const char *sdev, int speed);
int sio_close(void);
int sio_set_speed(int speed);
int sio_send(char *data, int len);
int sio_receive(char *data, int len);


#endif  /* __sio_h */
