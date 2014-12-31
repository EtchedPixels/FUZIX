/* lklib.c

   Copyright (C) 1989-1995 Alan R. Baldwin
   721 Berkeley St., Kent, Ohio 44240
   Copyright (C) 2008-2009 Borut Razem, borut dot razem at siol dot net

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

/*
 * With contributions for the
 * object libraries from
 * Ken Hornstein
 * kenh@cmf.nrl.navy.mil
 *
 */

/*
 * Extensions: P. Felber
 */

#include <string.h>

#include "sdld.h"
#include "lk_readnl.h"
#include "aslink.h"
#include "lklibr.h"
#include "lkrel.h"

static int
is_lib (FILE * libfp)
{
  return 1;
}

#ifdef INDEXLIB
/* buildlibraryindex - build an in-memory cache of the symbols contained in
 *                     the libraries
 */
static pmlibraryfile
buildlibraryindex_lib (struct lbname *lbnh, FILE * libfp, pmlibraryfile This, int type)
{
  char relfil[NINPUT];

  while (lk_readnl (relfil, sizeof (relfil), libfp) != NULL)
    {
      FILE *fp;
      char str[PATH_MAX];

      if (lbnh->path != NULL)
        {
          strcpy (str, lbnh->path);
#ifdef  OTHERSYSTEM
          if ((*str != '\0') && (str[strlen (str) - 1] != '/') && (str[strlen (str) - 1] != LKDIRSEP))
            {
              strcat (str, LKDIRSEPSTR);
            }
#endif
        }
      else
        str[0] = '\0';

      if ((relfil[0] == '/') || (relfil[0] == LKDIRSEP))
        {
          strcat (str, relfil + 1);
        }
      else
        {
          strcat (str, relfil);
        }

      if (strchr (relfil, FSEPX) == NULL)
        {
          sprintf (&str[strlen (str)], "%c%s", FSEPX, LKOBJEXT);
        }

      if ((fp = fopen (str, "rb")) != NULL)
        {
          /* Opened OK - create a new libraryfile object for it */
          if (libr == NULL)
            {
              libr = This = (pmlibraryfile) new (sizeof (mlibraryfile));
            }
          else
            {
              This->next = (pmlibraryfile) new (sizeof (mlibraryfile));
              This = This->next;
            }
          This->next = NULL;
          This->loaded = -1;
          This->libspc = lbnh->libspc;
          This->relfil = strdup (relfil);
          This->filspc = strdup (str);
          This->type = type;

          /* Start a new linked list of symbols for this module: */
          This->symbols = NULL;

          add_rel_index (fp, -1, This);
          fclose (fp);
        }                       /* Closes if object file opened OK */
      else
        {
          fprintf (stderr, "?ASlink-Warning-Cannot open library module %s\n", str);
        }
    }                           /* Ends while - processing all in libr */

  return This;
}

#else

static int
fndsym_lib (const char *name, struct lbname *lbnh, FILE * libfp, int type)
{
  char relfil[NINPUT];

  D ("Searching symbol: %s\n", name);

  while (lk_readnl (relfil, sizeof (relfil), libfp) != NULL)
    {
      char str[PATH_MAX];
      FILE *fp;

      if (lbnh->path != NULL)
        {
          strcpy (str, lbnh->path);
#ifdef  OTHERSYSTEM
          if ((*str != '\0') && (str[strlen (str) - 1] != '/') && (str[strlen (str) - 1] != LKDIRSEP))
            {
              strcat (str, LKDIRSEPSTR);
            }
#endif
        }
      else
        str[0] = '\0';

      if ((relfil[0] == '/') || (relfil[0] == LKDIRSEP))
        {
          strcat (str, relfil + 1);
        }
      else
        {
          strcat (str, relfil);
        }

      if (strchr (relfil, FSEPX) == NULL)
        {
          sprintf (&str[strlen (str)], "%c%s", FSEPX, LKOBJEXT);
        }

      if ((fp = fopen (str, "rb")) != NULL)
        {
          /* Opened OK - create a new libraryfile object for it */
          int ret = add_rel_file (name, lbnh, relfil, str, -1, fp, -1, type);
          fclose (fp);
          if (ret)
            {
              D ("Loaded module %s from file %s.\n", str, str);
              /* if cdb information required & adb file present */
              if (yflag && yfp)
                {
                  FILE *xfp = afile (str, "adb", 0);    //JCF: Nov 30, 2002
                  if (xfp)
                    {
                      SaveLinkedFilePath (str);
                      copyfile (yfp, xfp);
                      fclose (xfp);
                    }
                }
              return 1;         /* Found the symbol, so success! */
            }
        }                       /* Closes if object file opened OK */
      else
        {
          fprintf (stderr, "?ASlink-Warning-Cannot open library module %s\n", str);
        }
    }                           /* Ends while - processing all in libr */

  return 0;                     /* The symbol is not in this library */
}
#endif

/*)Function VOID    loadfile_lib(filspc)
 *
 *      char    *filspc     library object file specification
 *
 *  The function loadfile() links the library object module.
 *
 *  local variables:
 *      FILE    *fp         file handle
 *      int     i           input line length
 *      char    str[]       file input line
 *
 *  global variables:
 *      char    *ip         pointer to linker input string
 *
 *   functions called:
 *      int     fclose()    c_library
 *      char    *lk_readnl()  lk_readnl.c
 *      FILE *  fopen()     c_library
 *      VOID    link_main() lkmain.c
 *      int     strlen()    c_library
 *
 *  side effects:
 *      If file exists it is linked.
 */

static VOID
loadfile_lib (struct lbfile *lbfh)
{
  FILE *fp;
#ifdef __CYGWIN__
  char posix_path[PATH_MAX];
  void cygwin_conv_to_full_posix_path (char *win_path, char *posix_path);
  cygwin_conv_to_full_posix_path (lbfh->filspc, posix_path);
  fp = fopen (posix_path, "rb");
#else
  fp = fopen (lbfh->filspc, "rb");
#endif

  if (fp != NULL)
    {
      load_rel (fp, -1);
      fclose (fp);
    }
  else
    {
      fprintf (stderr, "?ASlink-Error-Opening library '%s'\n", lbfh->filspc);
      fclose (fp);
      lkexit (1);
    }
}

struct aslib_target aslib_target_lib = {
  &is_lib,
#ifdef INDEXLIB
  &buildlibraryindex_lib,
#else
  &fndsym_lib,
#endif
  &loadfile_lib,
};
