/*
 * Largely rewritten 2020 Alan Cox from the old ELKS code in order to address
 * the incorrect behaviour of the ELKS one
 *
 * --------------------------------------------------------------------------
 *
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

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static unsigned long do_conv(const char *nptr, char **endptr, int base, int uns)
{
	unsigned long int number = 0;
	unsigned long int newv;
	uint8_t negative = 0;
	uint8_t overflow = 0;

	/* Sanity check the arguments */
	if (base == 1 || base > 36 || base < 0) {
		errno = EINVAL;
		goto done;
	}

	/* advance beyond any leading whitespace */
	while (isspace(*nptr))
		nptr++;

	/* check for optional '+' or '-' */
	if (*nptr == '-') {
		negative = 1;
		nptr++;
	} else if (*nptr == '+')
		nptr++;

	/* The rules are a bit odd
	   base = 0 means 0x is hex 0nnn is octal
	   base = 16 means 0x is allowed but does nothing */
	if (*nptr == '0') {
		if ((base == 0 || base == 16) && (nptr[1] == 'X' || nptr[1] == 'x')) {
			base = 16;
			nptr += 2;
		}
		if (base == 0) {
			base = 8;
			nptr++;
		}
	}
	/* If base is still 0 (it was 0 to begin with and the string didn't begin
	   with "0"), then we are supposed to assume that it's base 10 */
	if (base == 0)
		base = 10;

	/* Now walk through the characters converting */
	while (isascii(*nptr) && isalnum(*nptr)) {
		uint8_t ch = toupper(*nptr);
		ch -= (ch <= '9' ? '0' : 'A' - 10);
		if (ch > base)
			break;
		/* Check for unsigned overflow */
		newv = (number * base) + ch;
		if (newv < number)
			overflow = 1;
		number = newv; 
		nptr++;
	}
done:
	/* Some code is simply _impossible_ to write with -Wcast-qual .. :-\ */
	if (endptr != NULL)
		*endptr = (char *) nptr;
	/* If we overflowed then return the right code */
	if (overflow) {
		errno = ERANGE;
		if (uns)
			return ULONG_MAX;
		if (negative)
			return LONG_MIN;
		return LONG_MAX;
	}
	/* For signed cases check that even if we didn't unsigned overflow
	   our final return will not break */
	if (!uns) {
		if (number == LONG_MIN && negative) {
			errno = ERANGE;
			return LONG_MIN;
		}
		if (number > LONG_MAX) {
			errno = ERANGE;
			number = LONG_MAX;
		}
	}
	/* All done */
	return negative ? -number : number;
}

long strtol(const char *nptr, char **endptr, int base)
{
	return (long)do_conv(nptr, endptr, base, 1);
}

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
	return do_conv(nptr, endptr, base, 0);
}


