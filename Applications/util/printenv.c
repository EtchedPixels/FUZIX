/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
    char **env;
    extern char **environ;
    int len;

    env = environ;

    if (argc == 1) {
	while (*env)
	    printf("%s\n", *env++);
	return 0;
    }

    len = strlen(argv[1]);
    while (*env) {
	if ((strlen(*env) > len) && (env[0][len] == '=') &&
	    (memcmp(argv[1], *env, len) == 0)) {
	    printf("%s\n", &env[0][len + 1]);
	    return 0;
	}
	env++;
    }
    
    return 1;
}
