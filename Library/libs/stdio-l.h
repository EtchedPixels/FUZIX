#ifndef _STDIO_L_H
#define _STDIO_L_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <types.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifndef STATIC
#define STATIC
#endif

extern FILE *__IO_list;         /* For fflush at exit */

void __stdio_init_vars(void);

#endif
