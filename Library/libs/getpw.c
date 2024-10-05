/*
 * getpw.c - This file is part of the libc-8086/pwd package for ELKS,
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

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>

int
getpw(uid_t uid, char *buf)
{
  register struct passwd * passwd;

  if (buf==NULL)
    {
      errno=EINVAL;
      return -1;
    }
  if ((passwd=getpwuid(uid))==NULL) {
    errno=ENOENT;
    return -1;
  }

  if (sprintf(buf, "%s:%s:%u:%u:%s:%s:%s", passwd->pw_name, passwd->pw_passwd,
	  passwd->pw_gid, passwd->pw_uid, passwd->pw_gecos,
	  passwd->pw_dir, passwd->pw_shell)<0)
    {
      errno=ENOMEM;
      return -1;
    }

  return 0;
}  



