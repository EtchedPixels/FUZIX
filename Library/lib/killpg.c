/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>

int killpg(pid_t pid, int sig)
{
   if(pid == 0)
       pid = getpgrp();
   if(pid > 1)
       return kill(-pid, sig);
   errno = EINVAL;
   return -1;
}
