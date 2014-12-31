/* lklibr.h

   Copyright (C) 1989-1995 Alan R. Baldwin
   721 Berkeley St., Kent, Ohio 44240
   Copyright (C) 2008 Borut Razem, borut dot razem at siol dot net

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

#ifndef __LKLIBR_H
#define __LKLIBR_H

#include <stdio.h>

#ifdef INDEXLIB
typedef struct slibrarysymbol mlibrarysymbol;
typedef struct slibrarysymbol *pmlibrarysymbol;

struct slibrarysymbol
{
  char *name;                   /* Warning: allocate memory before using */
  pmlibrarysymbol next;
};

typedef struct slibraryfile mlibraryfile;
typedef struct slibraryfile *pmlibraryfile;

struct slibraryfile
{
  int loaded;
  char *libspc;
  char *relfil;                 /* Warning: allocate memory before using */
  char *filspc;                 /* Warning: allocate memory before using */
  long offset;                  /* The embedded file offset in the library file libspc */
  unsigned int type;
  pmlibrarysymbol symbols;
  pmlibraryfile next;
};

extern pmlibraryfile libr;

pmlibrarysymbol add_rel_index (FILE * fp, long size, pmlibraryfile This);
#else
int is_module_loaded (const char *filspc);
int add_rel_file (const char *name, struct lbname *lbnh, const char *relfil,
                  const char *filspc, int offset, FILE * fp, long size, int type);
#endif

struct aslib_target
{
  int (*is_lib) (FILE * libfp);
#ifdef INDEXLIB
  pmlibraryfile (*buildlibraryindex) (struct lbname * lbnh, FILE * libfp, pmlibraryfile This, int type);
#else
  int (*fndsym) (const char *name, struct lbname * lbnh, FILE * libfp, int type);
#endif
  void (*loadfile) (struct lbfile * lbfh);
};

extern struct aslib_target aslib_target_sdcclib;
extern struct aslib_target aslib_target_ar;
extern struct aslib_target aslib_target_lib;

////
//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
# define D  printf
#else
# define D  1 ? (void)0 : (*(void (*)(const char *, ...))0)
#endif

#endif /* __LKLIBR_H */
