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


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>

int main(int argc, char *argv[])
{
	register struct utmp *entry;
	register char *timestr;
	char *p;
	register uint_fast8_t fmt = 0;

	p = strchr(argv[0],'/');
	if (p)
		p++;
	else
		p = argv[0];
	if (strcmp(p, "users") == 0)
		fmt = 1;
	setutent();
	while ((entry = getutent()) != NULL) {
		if (entry->ut_type == USER_PROCESS) {
			if (fmt == 0) {
				timestr = ctime(&entry->ut_time);
				timestr[strlen(timestr) - 1] = '\0';
				printf("%s	tty%c%c	%s %s\n", entry->ut_user, entry->ut_id[0], entry->ut_id[1] ? entry->ut_id[1] : ' ', timestr, entry->ut_host);
			} else {
				if (fmt == 2)
					putchar(' ');
				printf("%s", entry->ut_user);
				fmt = 2;
			}
		}
	}
	if (fmt == 2)
		putchar('\n');
	exit(0);
}
