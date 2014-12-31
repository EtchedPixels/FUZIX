/*
  lk_readnl.c - read a line from file into a buffer
  version 1.0.0, April 25th, 2008

  Copyright (c) 2008 Borut Razem

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Borut Razem
  borut.razem@siol.net
*/

#include "lk_readnl.h"

/*******************************************************************************

                                lk_readnl

lk_readnl() reads in at most one less than size characters from stream and stores
them into the buffer pointed to by s. Reading stops after an EOF or a newline.
The newline character is not stored into the buffer. A '\0' is stored after the
last character in the buffer. All the characters between size and the newline or
EOF are skipped.

lk_readnl() return s on success, and NULL on error or when end of file occurs
while no characters have been read.

*******************************************************************************/

char *
lk_readnl (char *s, int size, FILE * stream)
{
  static char eof_f = 0;
  int c = '\0';
  char *s_o;
  char prev_c;

  if (eof_f)
    {
      eof_f = 0;
      return NULL;
    }

  s_o = s;
  --size;                       /* for null terminator */
  while (size > 0)
    {
      prev_c = c;
      if ((c = getc (stream)) == '\n' || c == EOF)
        break;

      if (prev_c == '\r')
        {
          *s++ = prev_c;
          if (--size <= 0)
            break;
        }

      if (c != '\r')
        {
          *s++ = c;
          --size;
        }
    }
  *s = '\0';

  while (c != '\n' && c != EOF)
    c = getc (stream);

  if (c == EOF)
    {
      if (s == s_o)
        return NULL;

      eof_f = 1;
    }

  return s_o;
}
