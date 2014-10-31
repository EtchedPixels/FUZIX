/**************************** setenv.c ****************************/
/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int setenv(char *name, char *value, int replace)
{
	register char **ep;
	register size_t size = 0;
	size_t namelen = strlen(name);
	size_t vallen = strlen(value);
	char *p;
	ep = environ;
	while ((p = *ep++) != NULL) {
		if (!memcmp(p, name, namelen) && p[namelen] == '=')
			break;
		++size;
	}
	if (p == NULL) {
		static char **last_environ;
		char **new_environ =
		    (char **) malloc((size + 2) * sizeof(char *));
		if (new_environ == NULL) {
		      Err:errno = ENOMEM;
			return -1;
		}
		memcpy(new_environ, environ, size * sizeof(char *));
		if ((p = malloc(namelen + 1 + vallen + 1)) == NULL) {
			free(new_environ);
			goto Err;
		}
		memcpy(p, name, namelen);
		p[namelen] = '=';
		memcpy(&p[namelen + 1], value, vallen + 1);
		new_environ[size] = p;
		new_environ[size + 1] = NULL;
		if (last_environ != NULL)
			free((void *) last_environ);
		last_environ = new_environ;
		environ = new_environ;
	} else if (replace) {
		/* var exists and replaceing it contents is desired */
		size_t len = strlen(p);

		/* no room for new var and its contents. alloc space
		   for name and new content and copy them */
		if (len < namelen + 1 + vallen) {
			char *new = malloc(namelen + 1 + vallen + 1);
			if (new == NULL)
				goto Err;
			memcpy(new, name, namelen);	/* name */
			new[namelen] = '=';
			*--ep = p = new;
			/* next step: put new content */
		}

		/* if len(old_value)>len(new_value), so we can just
		   copy the new value over the old one */
		memcpy(&p[namelen + 1], value, vallen + 1);
	}
	return 0;
}

void unsetenv(char *name)
{
	register char **ep, **dp, *p;
	size_t namelen = strlen(name);
	dp = ep = environ;
	while ((p = *ep++) != NULL) {
		if (memcmp(p, name, namelen) || p[namelen] != '=')
			*dp++ = p;
	}
	*dp = NULL;
}
