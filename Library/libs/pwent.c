/*
 * pwent.c - This file is part of the libc-8086/pwd package for ELKS,
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

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>

/*
 * setpwent(), endpwent(), and getpwent() are included in the same object
 * file, since one cannot be used without the other two, so it makes sense to
 * link them all in together.
 */

/* file descriptor for the password file currently open */
static int pw_fd = -1;

void
setpwent(void)
{
  if (pw_fd!=-1)
    close(pw_fd);

  pw_fd=open("/etc/passwd", O_RDONLY);
}

void
endpwent(void)
{
  if (pw_fd!=-1)
    close(pw_fd);
  pw_fd=-1;
}

struct passwd *
getpwent(void)
{
  if (pw_fd==-1)
    setpwent();
  if (pw_fd!=-1)
    return __getpwent(pw_fd);
  return NULL;  
}


 
