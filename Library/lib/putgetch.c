/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
/* This is an implementation of the C standard IO package. */ 
    
#include "stdio-l.h"

int _putchar(int ch) 
{
	return write(STDOUT_FILENO, (char *)&ch, 1);
}

int _getchar(void)
{
	unsigned char ch;
	if (read(STDIN_FILENO, &ch, 1) == 1)
		return ch;
	return EOF;
}
