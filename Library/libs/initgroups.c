/*
 * initgroups.c - This file is part of the libc-8086/grp package for ELKS,
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
#include <string.h>
#include <fcntl.h>
#include <grp.h>
#include "config-getent.h"

int
initgroups(__const char * user, gid_t gid)
{
  register struct group * group;
#ifndef GR_DYNAMIC_GROUP_LIST
  gid_t group_list[GR_MAX_GROUPS];
#else
  gid_t * group_list=NULL;
#endif
  register char ** tmp_mem;
  int num_groups;
  int grp_fd;


  if ((grp_fd=open("/etc/group", O_RDONLY))<0)
    return -1;

  num_groups=0;
#ifdef GR_DYNAMIC_GROUP_LIST
  group_list=(gid_t *) realloc(group_list, 1);
#endif
  group_list[num_groups]=gid;
#ifndef GR_DYNAMIC_GROUP_LIST
  while (num_groups<GR_MAX_GROUPS &&
	 (group=__getgrent(grp_fd))!=NULL)
#else
  while ((group=__getgrent(grp_fd))!=NULL)
#endif      
    {
      if (group->gr_gid!=gid);
        {
	  tmp_mem=group->gr_mem;
	  while(*tmp_mem!=NULL)
	    {
	      if (!strcmp(*tmp_mem, user))
		{
		  num_groups++;
#ifdef GR_DYNAMIC_GROUP_LIST  
		  group_list=(gid_t *)realloc(group_list,
					      num_groups*sizeof(gid_t *));
#endif		  
		  group_list[num_groups]=group->gr_gid;
		}
	      tmp_mem++;
	    }
	}
    }
  close(grp_fd);
  return setgroups(num_groups, group_list);
}




