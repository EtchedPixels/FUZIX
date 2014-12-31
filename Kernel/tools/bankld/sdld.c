/* sdld.c

   Copyright (C) 2009-2010 Borut Razem

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _WIN32
#include <libgen.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sdld.h"

#define NELEM(x)  (sizeof (x) / sizeof (x)[0])


static int sdld = -1;
static enum sdld_target_e target = TARGET_ID_UNKNOWN;


static char
*program_name (char *path)
{
#ifdef _WIN32
  static char fname[_MAX_FNAME];
  char *p;

  _splitpath (path, NULL, NULL, fname, NULL);
  /* convert it to lower case:
     on DOS and Windows 9x the file name in argv[0] is uppercase */
  for (p = fname; '\0' != *p; ++p)
    *p = tolower (*p);
  return fname;
#else
  return basename (path);
#endif
}


static void
check_init(void)
{
  if (sdld == -1)
    {
      fprintf(stderr, "sdld_init not called!\n");
      exit (1);
    }
}


void
sdld_init (char *path)
{
  struct tgt_s {
    char *str;
    enum sdld_target_e target;
  } tgt[] = {
    { "gb",   TARGET_ID_GB,   },
    { "z80",  TARGET_ID_Z80,  },
    { "z180", TARGET_ID_Z180, },
    { "8051", TARGET_ID_8051, },
    { "6808", TARGET_ID_6808, },
    { "stm8", TARGET_ID_STM8, },
  };
  int i = NELEM (tgt);

  char *progname = program_name (path);
  if ((sdld = (strncmp(progname, "sdld", 4) == 0)) != 0)
    {
      /* exception: sdld is 8051 linker */
      if (progname[4] == '\0')
        target = TARGET_ID_8051;
      else
        {
          for (i = 0; i < NELEM (tgt); ++i)
            {
              if (strstr(progname, tgt[i].str))
                {
                  target = tgt[i].target;
                  break;
                }
            }
        }
    }
  /* diagnostic message */
  if (getenv ("SDLD_DIAG"))
    {
      printf ("sdld path: %s\n", path);
      printf ("is sdld: %d\n", sdld);
      if (sdld)
        printf ("sdld target: %s\n", (i >= NELEM (tgt)) ? "8051" : tgt[i].str);
    }
}


int
is_sdld(void)
{
  check_init();
  return sdld;
}


enum sdld_target_e
get_sdld_target(void)
{
  check_init();
  return target;
}


int
is_sdld_target_z80_like(void)
{
  check_init();
  return target == TARGET_ID_Z80 || target == TARGET_ID_Z180 || target == TARGET_ID_GB;
}


int
is_sdld_target_8051_like(void)
{
  check_init();
  return target == TARGET_ID_8051;
}
