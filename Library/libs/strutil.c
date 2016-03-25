/*  supplemental C string library utilities for words and sentences
    Copyright (C) 2015 Erkin Alp Güney <erkinalp9035@gmail.com>
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation; version 2.1.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <strutil.h>
extern size_t strcnt(const char* string,char delim) {
/** strcnt-Count occurences of a given character in a string **/
    size_t i;
    for (i=0;*string;string++) if (*string==delim) i++;
    return i;
}

extern size_t strlnlen(const char* string) {
/** strlnlen-Return maximal line length in a given string **/
    size_t maxlen=0;
    for (size_t i=0;*string;string++) 
        if (*string=='\n'&&(maxlen<i)) {maxlen=i; i=0;} else i++;
    return maxlen;
}


