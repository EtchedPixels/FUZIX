/*
 * __getpwent.c - This file is part of the libc-8086/pwd package for ELKS,
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
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

#define PWD_BUFFER_SIZE 256

/* This isn't as flash as my previous version -- it doesn't dynamically
  scale down the gecos on too-long lines, but it also makes fewer syscalls,
  so it's probably nicer.  Write me if you want the old version.  Maybe I
  should include it as a build-time option... ?
  -Nat <ndf@linux.mit.edu> */
   
struct passwd *
__getpwent(int pwd_fd)
{
  static char line_buff[PWD_BUFFER_SIZE];
  static struct passwd passwd;
  char * field_begin;
  char * endptr;
  char * gid_ptr;
  char * uid_ptr;
  int line_len;
  int i;

  /* We use the restart label to handle malformatted lines */
restart:
  /* Read the passwd line into the static buffer using a minimal of
     syscalls. */
  if ((line_len=read(pwd_fd, line_buff, PWD_BUFFER_SIZE))<=0)
    return NULL;
  field_begin=strchr(line_buff, '\n');
  if (field_begin!=NULL)
    lseek(pwd_fd, (long) (1+field_begin-(line_buff+line_len)), SEEK_CUR);
  else /* The line is too long - skip it. :-\ */
    {
      do { if ((line_len=read(pwd_fd, line_buff, PWD_BUFFER_SIZE))<=0)
	return NULL;
      } while (!(field_begin=strchr(line_buff, '\n')));
      lseek(pwd_fd, (long) (field_begin-line_buff)-line_len+1, SEEK_CUR);
      goto restart;
    }
  if (*line_buff=='#' || *line_buff==' ' || *line_buff=='\n' ||
      *line_buff=='\t')
      goto restart;
  *field_begin='\0';

  /* We've read the line; now parse it. */
  field_begin=line_buff;
  for (i=0;i<7;i++)
    {
      switch(i)
	{
	case 0: passwd.pw_name=field_begin; break;
	case 1: passwd.pw_passwd=field_begin; break;
	case 2: uid_ptr=field_begin; break;
	case 3: gid_ptr=field_begin; break;
	case 4: passwd.pw_gecos=field_begin; break;
	case 5: passwd.pw_dir=field_begin; break;
	case 6: passwd.pw_shell=field_begin; break;
	}
      if (i<6)
	{
	  field_begin=strchr(field_begin, ':');
	  if (field_begin==NULL) goto restart;
	  *field_begin++='\0';
	}
    }
  passwd.pw_gid=(gid_t) strtoul(gid_ptr, &endptr, 10);
  if (*endptr!='\0') goto restart;
  
  passwd.pw_uid=(uid_t) strtoul(uid_ptr, &endptr, 10);
  if (*endptr!='\0') goto restart;

  return &passwd;
}


