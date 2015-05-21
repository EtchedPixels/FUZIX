/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Stdio usage removed Alan Cox 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writesnl(const char *p)
{
    write(1, p, strlen(p));
    write(1, "\n", 1);
}

int main(int argc, char *argv[])
{
    char **env;
    extern char **environ;
    int len;

    env = environ;

    if (argc == 1) {
	while (*env)
	    writesnl(*env++);
	return 0;
    }

    len = strlen(argv[1]);
    while (*env) {
	if ((strlen(*env) > len) && (env[0][len] == '=') &&
	    (memcmp(argv[1], *env, len) == 0)) {
	    writesnl(&env[0][len + 1]);
	    return 0;
	}
	env++;
    }
    
    return 1;
}
