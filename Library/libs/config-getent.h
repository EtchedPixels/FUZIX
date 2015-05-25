/*
 * config-grp.h - This file is part of the libc-8086/grp package for ELKS,
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


#ifndef _CONFIG_GRP_H
#define _CONFIG_GRP_H

/*
 * Define GR_SCALE_DYNAMIC if you want grp to dynamically scale its read buffer
 * so that lines of any length can be used.  On very very small systems,
 * you may want to leave this undefined becasue it will make the grp functions
 * somewhat larger (because of the inclusion of malloc and the code necessary).
 * On larger systems, you will want to define this, because grp will _not_
 * deal with long lines gracefully (they will be skipped).
 */
#undef GR_SCALE_DYNAMIC

#ifndef GR_SCALE_DYNAMIC
/*
 * If scaling is not dynamic, the buffers will be statically allocated, and
 * maximums must be chosen.  GR_MAX_LINE_LEN is the maximum number of
 * characters per line in the group file.  GR_MAX_MEMBERS is the maximum
 * number of members of any given group.
 */
#define GR_MAX_LINE_LEN 186
/* GR_MAX_MEMBERS = (GR_MAX_LINE_LEN-(24+3+6))/9 */
#define GR_MAX_MEMBERS 16

#endif /* !GR_SCALE_DYNAMIC */


/*
 * Define GR_DYNAMIC_GROUP_LIST to make initgroups() dynamically allocate
 * space for it's GID array before calling setgroups().  This is probably
 * unnecessary scalage, so it's undefined by default.
 */
#undef GR_DYNAMIC_GROUP_LIST

#ifndef GR_DYNAMIC_GROUP_LIST

#endif /* !GR_DYNAMIC_GROUP_LIST */

#endif /* !_CONFIG_GRP_H */
