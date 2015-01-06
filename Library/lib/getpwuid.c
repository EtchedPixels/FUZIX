/*
 * getpwuid.c - This file is part of the libc-8086/pwd package for ELKS,
 * Copyright (C) 1995, 1996 Nat Friedman <ndf@linux.mit.edu>.
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>

struct passwd *
getpwuid(uid_t uid)
{
  int passwd_fd;
  struct passwd * passwd;

  if ((passwd_fd=open("/etc/passwd", O_RDONLY))<0)
    return NULL;

  while ((passwd=__getpwent(passwd_fd))!=NULL)
    if (passwd->pw_uid==uid)
      {
	close(passwd_fd);
	return passwd;
      }

  close (passwd_fd);
  return NULL;
}
