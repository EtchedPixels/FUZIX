/* 

Commandline handling attempt.

*/


#include <kernel.h>

/* This stuff is directly from the standard C lib */

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  

//#include <string.h>

/* FIXME: asm version ?? */    
/********************** Function strchr ************************************/ 
char *strchr(const char *s, int c) 
{
        register char ch;
        
        for (;;) {
                if ((ch = *s) == c)
                        return s;
                if (ch == 0)
                        return 0;
                s++;
        }
}


char *strpbrk(const char *str, const char *set)
{
  while (*str != '\0')
    if (strchr(set, *str) == 0)
      ++str;
    else
      return (char *) str;

  return 0;
}

size_t strspn(const char *s, const char *accept)
{
  const char *p;
  const char *a;
  size_t count = 0;

  for (p = s; *p != '\0'; ++p)
    {
      for (a = accept; *a != '\0'; ++a)
        if (*p == *a)
          break;
      if (*a == '\0')
        return count;
      else
        ++count;
    }

  return count;
}
/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

//#include <string.h>


static char *olds = 0;
char *strtok(char *s, const char *delim)
{
  char *token;

  if (s == 0)
    {
      if (olds == 0)
        {
          return 0;
        }
      else
        s = olds;
    }

  /* Scan leading delimiters.  */
  s += strspn(s, delim);
  if (*s == '\0')
    {
      olds = 0;
      return 0;
    }
  /* Find the end of the token.  */
  token = s;
  s = strpbrk(token, delim);
  if (s == 0)
    /* This token finishes the string.  */
    olds = 0;
  else
    {
      /* Terminate the token and make OLDS point past it.  */
      *s = '\0';
      olds = s + 1;
    }
  return token;
}


/* END of standard C string functions */


char *kargv[32];
int kargc=0;


/* This parses the commandline into kargv and kargc */
void cmdline_init()
{
	char *p=strtok( CMDLINE, " ");
	if( p ){
		kargv[kargc++]=p;
		p=strtok( NULL, " ");
	}
	kargv[kargc]=NULL;
}
