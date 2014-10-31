/*
 * strtol.c - This file is part of the libc-8086 package for ELKS,
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

/* TODO: This needs range clamping and setting errno when it's done. */

#include <ctype.h>
#include <stdlib.h>

long int
strtol(const char *nptr, char **endptr, int base)
{
  const char * ptr;
  unsigned short negative;
  long int number;

  ptr=nptr;

  while (isspace(*ptr))
    ptr++;

  negative=0;
  if (*ptr=='-')
    negative=1;

  number=(long int)strtoul(nptr, endptr, base);

  return (negative ? -number:number);
}

unsigned long int
strtoul(const char *nptr, char **endptr, int base)
{
  unsigned long int number;

  /* Sanity check the arguments */
  if (base==1 || base>36 || base<0)
    base=0;
  
  /* advance beyond any leading whitespace */
  while (isspace(*nptr))
    nptr++;

  /* check for optional '+' or '-' */
  if (*nptr=='-')
      nptr++;
  else
    if (*nptr=='+')
      nptr++;

  /* If base==0 and the string begins with "0x" then we're supposed
     to assume that it's hexadecimal (base 16). */
  if (base==0 && *nptr=='0')
    {
      if (toupper(*(nptr+1))=='X')
	{
	  base=16;
	  nptr+=2;
	}
  /* If base==0 and the string begins with "0" but not "0x",
     then we're supposed to assume that it's octal (base 8). */
      else
	{
	  base=8;
	  nptr++;
	}
    }

  /* If base is still 0 (it was 0 to begin with and the string didn't begin
     with "0"), then we are supposed to assume that it's base 10 */
  if (base==0)
    base=10;

  number=0;
  while (isascii(*nptr) && isalnum(*nptr))
    {
      int ch = *nptr;
      if (islower(ch)) ch = toupper(ch);
      ch -= (ch<='9' ? '0' : 'A'-10);
      if (ch>base)
	break;

      number= (number*base)+ch;
      nptr++;
    }

  /* Some code is simply _impossible_ to write with -Wcast-qual .. :-\ */
  if (endptr!=NULL)
    *endptr=(char *)nptr;

  /* All done */
  return number;
}


