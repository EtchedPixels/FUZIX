/*
 * who.c
 *
 * Copyright 1998 Alistair Riddoch
 * ajr@ecs.soton.ac.uk
 *
 * This file may be distributed under the terms of the GNU General Public
 * License v2, or at your option any later version.
 */

/*
 * This is a small version of who for use in the ELKS project.
 * It is not fully functional, and may not be the most efficient
 * implementation for larger systems. It minimises memory usage and
 * code size.
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>

void main(int argc, char *argv[])
{
    register struct utmp * entry;
    char * timestr;

    setutent();
    while ((entry = getutent()) != NULL)
	if (entry->ut_type == USER_PROCESS) {
	    timestr = ctime(&entry->ut_time);
	    timestr[strlen(timestr) - 1] = '\0';
	    printf("%s	tty%c%c	%s %s\n", entry->ut_user,
			entry->ut_id[0],
			entry->ut_id[1] ? entry->ut_id[1] : 0,
			timestr,
			entry->ut_host);
	}
    exit(0);
}
