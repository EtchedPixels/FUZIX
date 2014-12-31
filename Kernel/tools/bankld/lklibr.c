/* lklibr.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 *
 * With contributions for the
 * object libraries from
 * Ken Hornstein
 * kenhat cmf dot nrl dot navy dot mil
 *
 */

/*
 * Extensions: P. Felber
 */

#include <ctype.h>
#include <assert.h>

#include "aslink.h"
#include "lkrel.h"
#include "lklibr.h"

/*)Module   lklibr.c
 *
 *  The module lklibr.c contains the functions which
 *  (1) specify the path(s) to library files [.LIB]
 *  (2) specify the library file(s) [.LIB] to search
 *  (3) search the library files for specific symbols
 *      and link the module containing this symbol
 *
 *  lklibr.c contains the following functions:
 *      VOID    addpath()
 *      VOID    addlib()
 *      VOID    addfile()
 *      VOID    search()
 *      VOID    fndsym()
 *      VOID    library()
 *      VOID    loadfile()
 *
 */

#define EQ(A,B) !strcmp((A),(B))
#define NELEM(x) (sizeof (x) / sizeof (*x))

#ifdef INDEXLIB
/* First entry in the library object symbol cache */
pmlibraryfile libr = NULL;

int buildlibraryindex (void);
void freelibraryindex (void);
#endif /* INDEXLIB */

struct aslib_target *aslib_targets[] = {
  &aslib_target_sdcclib,
  &aslib_target_ar,
  &aslib_target_lib,
};

/*)Function VOID    addpath()
 *
 *  The function addpath() creates a linked structure containing
 *  the paths to various object module library files.
 *
 *  local variables:
 *      lbpath  *lbph       pointer to new path structure
 *      lbpath  *lbp        temporary pointer
 *
 *  global variables:
 *      lbpath  *lbphead    The pointer to the first
 *                          path structure
 *
 *   functions called:
 *      int     getnb()     lklex.c
 *      VOID *  new()       lksym.c
 *      int     strlen()    c_library
 *      char *  strcpy()    c_library
 *      VOID    unget()     lklex.c
 *
 *  side effects:
 *      An lbpath structure may be created.
 */

VOID
addpath (void)
{
  struct lbpath *lbph, *lbp;

  lbph = (struct lbpath *) new (sizeof (struct lbpath));
  if (lbphead == NULL)
    {
      lbphead = lbph;
    }
  else
    {
      lbp = lbphead;
      while (lbp->next)
        {
          lbp = lbp->next;
        }
      lbp->next = lbph;
    }
  unget (getnb ());
  lbph->path = strdup (ip);
}

/*)Function VOID    addlib()
 *
 *  The function addlib() tests for the existance of a
 *  library path structure to determine the method of
 *  adding this library file to the library search structure.
 *
 *  This function calls the function addfile() to actually
 *  add the library file to the search list.
 *
 *  local variables:
 *      lbpath  *lbph       pointer to path structure
 *
 *  global variables:
 *      lbpath  *lbphead    The pointer to the first
 *                          path structure
 *      ip a pointer to the library name
 *
 *   functions called:
 *      VOID    addfile()   lklibr.c
 *      int     getnb()     lklex.c
 *      VOID    unget()     lklex.c
 *
 *  side effects:
 *      The function addfile() may add the file to
 *      the library search list.
 */

VOID
addlib (void)
{
  struct lbpath *lbph;
  int foundcount = 0;

  unget (getnb ());

  if (lbphead == NULL)
    {
      foundcount = addfile (NULL, ip);
    }
  else
    {
      for (lbph = lbphead; lbph; lbph = lbph->next)
        {
          foundcount += addfile (lbph->path, ip);
        }
    }
  if (foundcount == 0)
    {
      fprintf (stderr, "?ASlink-Warning-Couldn't find library '%s'\n", ip);
    }
}

/*)Function int addfile(path,libfil)
 *
 *      char    *path       library path specification
 *      char    *libfil     library file specification
 *
 *  The function addfile() searches for the library file
 *  by concatenating the path and libfil specifications.
 *  if the library is found, an lbname structure is created
 *  and linked to any previously defined structures.  This
 *  linked list is used by the function fndsym() to attempt
 *  to find any undefined symbols.
 *
 *  The function does not report an error on invalid
 *  path / file specifications or if the file is not found.
 *
 *  local variables:
 *      lbname  *lbnh           pointer to new name structure
 *      lbname  *lbn            temporary pointer
 *      char *  str             path / file string
 *      char *  strend          end of path pointer
 *      char *  str             path / file string
 *      char *  strend          end of path pointer
 *
 *  global variables:
 *      lbname  *lbnhead        The pointer to the first
 *                              path structure
 *      int     objflg          linked file/library object output flag
 *
 *   functions called:
 *      int     getnb()     lklex.c
 *      VOID *  new()       lksym.c
 *      int     strlen()    c_library
 *      char *  strcpy()    c_library
 *      VOID    unget()     lklex.c
 *
 *  side effects:
 *      An lbname structure may be created.
 *
 *  return:
 *      1: the library was found
 *      0: the library was not found
 */

int
addfile (char *path, char *libfil)
{
  FILE *fp;
  char *str, *strend;
  struct lbname *lbnh, *lbn;
#ifdef  OTHERSYSTEM
  int libfilinc = 0;
#endif

  if (path != NULL)
    {
      str = (char *) new (strlen (path) + strlen (libfil) + 6);
      strcpy (str, path);
      strend = str + strlen(str) - 1;
#ifdef  OTHERSYSTEM
      if (strlen (str) && (*strend != '/') && (*strend != LKDIRSEP))
        {
          strcat (str, LKDIRSEPSTR);
        }
#endif
    }
  else
    {
      str = (char *) new (strlen (libfil) + 5);
    }

#ifdef  OTHERSYSTEM
  if ((libfil[0] == '/') || (libfil[0] == LKDIRSEP))
    {
      libfil++;
      libfilinc = 1;
    }
#endif

  strcat (str, libfil);
  if (strchr (libfil, FSEPX) == NULL)
    {
      sprintf (&str[strlen (str)], "%clib", FSEPX);
    }

  fp = fopen (str, "rb");
  if (fp == NULL)
    {
      /*Ok, that didn't work.  Try with the 'libfil' name only */
#ifdef  OTHERSYSTEM
      if (libfilinc)
        libfil--;
#endif
      fp = fopen (libfil, "rb");
      if (fp != NULL)
        {
          /*Bingo!  'libfil' is the absolute path of the library */
          strcpy (str, libfil);
          path = NULL;          /*This way 'libfil' and 'path' will be rebuilt from 'str' */
        }
    }

  if (path == NULL)
    {
      /*'path' can not be null since it is needed to find the object files associated with
         the library.  So, get 'path' from 'str' and then chop it off and recreate 'libfil'.
         That way putting 'path' and 'libfil' together will result into the original filepath
         as contained in 'str'. */
      int j;
      path = strdup (str);
      for (j = strlen (path) - 1; j >= 0; j--)
        {
          if ((path[j] == '/') || (path[j] == LKDIRSEP))
            {
              strcpy (libfil, &path[j + 1]);
              path[j + 1] = 0;
              break;
            }
        }
      if (j <= 0)
        path[0] = 0;
    }

  if (fp != NULL)
    {
      fclose (fp);
      lbnh = (struct lbname *) new (sizeof (struct lbname));
      if (lbnhead == NULL)
        {
          lbnhead = lbnh;
        }
      else
        {
          lbn = lbnhead;
          while (lbn->next)
            {
              lbn = lbn->next;
            }
          lbn->next = lbnh;
        }

      lbnh->path = path;
      lbnh->libfil = strdup (libfil);
      lbnh->libspc = str;
      lbnh->f_obj = objflg;
      return 1;
    }
  else
    {
      free (str);
      return 0;
    }
}

/*)Function VOID    search()
 *
 *  The function search() looks through all the symbol tables
 *  at the end of pass 1.  If any undefined symbols are found
 *  then the function fndsym() is called. Function fndsym()
 *  searches any specified library files to automagically
 *  import the object modules containing the needed symbol.
 *
 *  After a symbol is found and imported by the function
 *  fndsym() the symbol tables are again searched.  The
 *  symbol tables are searched until no more symbols can be
 *  resolved within the library files.  This ensures that
 *  back references from one library module to another are
 *  also resolved.
 *
 *  local variables:
 *      int     i           temporary counter
 *      sym     *sp         pointer to a symbol structure
 *      int     symfnd      found a symbol flag
 *
 *  global variables:
 *      sym     *symhash[]  array of pointers to symbol tables
 *
 *   functions called:
 *      int     fndsym()    lklibr.c
 *
 *  side effects:
 *      If a symbol is found then the library object module
 *      containing the symbol will be imported and linked.
 */

VOID
search (void)
{
  struct sym *sp;
  int i, symfnd;

  /*
   * Look for undefined symbols.  Keep
   * searching until no more symbols are resolved.
   */
  symfnd = 1;
  while (symfnd)
    {
      symfnd = 0;
      /*
       * Look through all the symbols
       */
      for (i = 0; i < NHASH; ++i)
        {
          sp = symhash[i];
          while (sp)
            {
              /* If we find an undefined symbol
               * (one where S_DEF is not set), then
               * try looking for it.  If we find it
               * in any of the libraries then
               * increment symfnd.  This will force
               * another pass of symbol searching and
               * make sure that back references work.
               */
              if ((sp->s_type & S_DEF) == 0)
                {
                  if (fndsym (sp->s_id))
                    {
                      symfnd++;
                    }
                }
              sp = sp->s_sp;
            }
        }
    }
}

/*)Function VOID    fndsym(name)
 *
 *      char    *name       symbol name to find
 *
 *  The function fndsym() searches through all combinations of the
 *  library path specifications (input by the -k option) and the
 *  library file specifications (input by the -l option) that
 *  lead to an existing file.
 *
 *  The file specification may be formed in one of two ways:
 *
 *  (1) If the library file contained an absolute
 *      path/file specification then this becomes filspc.
 *      (i.e. C:\...)
 *
 *  (2) If the library file contains a relative path/file
 *      specification then the concatenation of the path
 *      and this file specification becomes filspc.
 *      (i.e. \...)
 *
 *  The structure lbfile is created for the first library
 *  object file which contains the definition for the
 *  specified undefined symbol.
 *
 *  If the library file [.LIB] contains file specifications for
 *  non existant files, no errors are returned.
 *
 *  local variables:
 *      char    buf[]       [.REL] file input line
 *      char    c           [.REL] file input character
 *      FILE    *fp         file handle for object file
 *      lbfile  *lbf        temporary pointer
 *      lbfile  *lbfh       pointer to lbfile structure
 *      FILE    *libfp      file handle for library file
 *      lbname  *lbnh       pointer to lbname structure
 *      char    *path       file specification path
 *      char    relfil[]    [.REL] file specification
 *      char    *str        combined path and file specification
 *      char    symname[]   [.REL] file symbol string
 *
 *  global variables:
 *      lbname  *lbnhead    The pointer to the first
 *                          name structure
 *      lbfile  *lbfhead    The pointer to the first
 *                          file structure
 *      int     obj_flag    linked file/library object output flag
 *
 *   functions called:
 *      int     fclose()    c_library
 *      FILE    *fopen()    c_library
 *      VOID    free()      c_library
 *      int     getnb()     lklex.c
 *      VOID    lkexit()    lkmain.c
 *      VOID    loadfile()  lklibr.c
 *      VOID *  new()       lksym.c
 *      char *  sprintf()   c_library
 *      int     sscanf()    c_library
 *      char *  strcat()    c_library
 *      char *  strchr()    c_library
 *      char *  strcpy()    c_library
 *      int     strlen()    c_library
 *      int     strncmp()   c_library
 *      VOID    unget()     lklex.c
 *
 *  side effects:
 *      If the symbol is found then a new lbfile structure
 *      is created and added to the linked list of lbfile
 *      structures.  The file containing the found symbol
 *      is linked.
 */

#ifdef INDEXLIB
int
fndsym (char *name)
{
  struct lbfile *lbfh, *lbf;
  pmlibraryfile ThisLibr;
  pmlibrarysymbol ThisSym = NULL;

  pmlibraryfile FirstFound;
  int numfound = 0;

  D ("Searching symbol: %s\n", name);

  /* Build the index if this is the first call to fndsym */
  if (libr == NULL)
    buildlibraryindex ();

  /* Iterate through all library object files */
  FirstFound = libr;            /* So gcc stops whining */
  for (ThisLibr = libr; ThisLibr != NULL; ThisLibr = ThisLibr->next)
    {
      /* Iterate through all symbols in an object file */
      for (ThisSym = ThisLibr->symbols; ThisSym != NULL; ThisSym = ThisSym->next)
        {
          if (!strcmp (ThisSym->name, name))
            {
              if ((!ThisLibr->loaded) && (numfound == 0))
                {
                  /* Object file is not loaded - add it to the list */
                  lbfh = (struct lbfile *) new (sizeof (struct lbfile));
                  if (lbfhead == NULL)
                    {
                      lbfhead = lbfh;
                    }
                  else
                    {
                      for (lbf = lbfhead; lbf->next != NULL; lbf = lbf->next)
                        ;

                      lbf->next = lbfh;
                    }
                  lbfh->libspc = ThisLibr->libspc;
                  lbfh->filspc = ThisLibr->filspc;
                  lbfh->relfil = strdup (ThisLibr->relfil);
                  lbfh->offset = ThisLibr->offset;
                  lbfh->type = ThisLibr->type;

                  (*aslib_targets[lbfh->type]->loadfile) (lbfh);

                  ThisLibr->loaded = 1;
                }

              if (numfound == 0)
                {
                  numfound++;
                  FirstFound = ThisLibr;
                }
              else
                {
                  char absPath1[PATH_MAX];
                  char absPath2[PATH_MAX];
#if defined(_WIN32)
                  int j;

                  _fullpath (absPath1, FirstFound->libspc, PATH_MAX);
                  _fullpath (absPath2, ThisLibr->libspc, PATH_MAX);
                  for (j = 0; absPath1[j] != 0; j++)
                    absPath1[j] = tolower ((unsigned char) absPath1[j]);
                  for (j = 0; absPath2[j] != 0; j++)
                    absPath2[j] = tolower ((unsigned char) absPath2[j]);
#else
                  if (NULL == realpath (FirstFound->libspc, absPath1))
                    *absPath1 = '\0';
                  if (NULL == realpath (ThisLibr->libspc, absPath2))
                    *absPath2 = '\0';
#endif
                  if (!(EQ (absPath1, absPath2) && EQ (FirstFound->relfil, ThisLibr->relfil)))
                    {
                      if (numfound == 1)
                        {
                          fprintf (stderr, "?ASlink-Warning-Definition of public symbol '%s'" " found more than once:\n", name);
                          fprintf (stderr, "   Library: '%s', Module: '%s'\n", FirstFound->libspc, FirstFound->relfil);
                        }
                      fprintf (stderr, "   Library: '%s', Module: '%s'\n", ThisLibr->libspc, ThisLibr->relfil);
                      numfound++;
                    }
                }
            }
        }
    }
  return numfound;
}

struct add_sym_s
{
  pmlibraryfile plf;
  pmlibrarysymbol pls;
};

static int
add_sybmol (const char *sym, void *param)
{
  struct add_sym_s *as = (struct add_sym_s *) param;
  pmlibrarysymbol ps = (pmlibrarysymbol) new (sizeof (mlibrarysymbol));

  D ("    Indexing symbol: %s\n", sym);

  as->plf->loaded = 0;
  ps->next = NULL;
  ps->name = strdup (sym);

  if (as->pls == NULL)
    {
      as->pls = as->plf->symbols = ps;
    }
  else
    {
      as->pls->next = ps;
      as->pls = as->pls->next;
    }

  return 0;
}

pmlibrarysymbol
add_rel_index (FILE * fp, long size, pmlibraryfile This)
{
  struct add_sym_s as;
  as.plf = This;
  as.pls = This->symbols;

  assert (This->symbols == NULL);

  enum_symbols (fp, size, &add_sybmol, &as);

  return as.pls;
}

/* buildlibraryindex - build an in-memory cache of the symbols contained in
 *                     the libraries
 */
int
buildlibraryindex (void)
{
  pmlibraryfile This = NULL;
  struct lbname *lbnh;

  /*
   * Search through every library in the linked list "lbnhead".
   */
  for (lbnh = lbnhead; lbnh; lbnh = lbnh->next)
    {
      FILE *libfp;
      int i;

      D ("Indexing library: %s\n", lbnh->libspc);

      if ((libfp = fopen (lbnh->libspc, "rb")) == NULL)
        {
          fprintf (stderr, "?ASlink-Error-Cannot open library file %s\n", lbnh->libspc);
          lkexit (1);
        }

      for (i = 0; i < NELEM (aslib_targets); ++i)
        {
          if ((*aslib_targets[i]->is_lib) (libfp))
            {
              This = (*aslib_targets[i]->buildlibraryindex) (lbnh, libfp, This, i);
              break;
            }
        }

      if (i >= NELEM (aslib_targets))
        fprintf (stderr, "?ASlink-Error-Unknown library file format %s\n", lbnh->libspc);

      fclose (libfp);
    }

  return 0;
}

/* Release all memory allocated for the in-memory library index */
void
freelibraryindex (void)
{
  pmlibraryfile ThisLibr, ThisLibr2Free;
  pmlibrarysymbol ThisSym, ThisSym2Free;

  ThisLibr = libr;

  while (ThisLibr)
    {
      ThisSym = ThisLibr->symbols;

      while (ThisSym)
        {
          free (ThisSym->name);
          ThisSym2Free = ThisSym;
          ThisSym = ThisSym->next;
          free (ThisSym2Free);
        }
      free (ThisLibr->filspc);
      free (ThisLibr->relfil);
      ThisLibr2Free = ThisLibr;
      ThisLibr = ThisLibr->next;
      free (ThisLibr2Free);
    }

  libr = NULL;
}

#else /* INDEXLIB */

struct load_sym_s
{
  const char *name;
  struct lbname *lbnh;
  const char *relfil;
  const char *filspc;
  int offset;
  int type;
};

static int
load_sybmol (const char *sym, void *params)
{
  struct load_sym_s *ls = (struct load_sym_s *) params;

  D ("    Symbol: %s\n", sym);

  if (strcmp (ls->name, sym) == 0)
    {
      struct lbfile *lbfh, *lbf;

      D ("    Symbol %s found in module %s!\n", sym, ls->relfil);

      lbfh = (struct lbfile *) new (sizeof (struct lbfile));
      lbfh->libspc = ls->lbnh->libspc;
      lbfh->relfil = strdup (ls->relfil);
      lbfh->filspc = strdup (ls->filspc);
      lbfh->offset = ls->offset;
      lbfh->type = ls->type;

      if (lbfhead == NULL)
        lbfhead = lbfh;
      else
        {
          for (lbf = lbfhead; lbf->next != NULL; lbf = lbf->next)
              ;
          lbf->next = lbfh;
        }
      (*aslib_targets[ls->type]->loadfile) (lbfh);

      return 1;
    }
  else
    return 0;
}

/*)Function int is_module_loaded(filspc)
 *
 * If this module has been already loaded
 */

int
is_module_loaded (const char *filspc)
{
  struct lbfile *lbf;

  for (lbf = lbfhead; lbf != NULL; lbf = lbf->next)
    {
      if (EQ (filspc, lbf->filspc))
        {
          D ("  Module %s already loaded!\n", filspc);
          return 1;       /* Module already loaded */
        }
    }
  return 0;
}

int
add_rel_file (const char *name, struct lbname *lbnh, const char *relfil,
              const char *filspc, int offset, FILE * fp, long size, int type)
{
  struct load_sym_s ls;

  /* If this module has been loaded already don't load it again. */
  if (is_module_loaded (filspc))
    return 0;
  else
    {
      ls.name = name;
      ls.lbnh = lbnh;
      ls.relfil = relfil;
      ls.filspc = filspc;
      ls.offset = offset;
      ls.type = type;

      return enum_symbols (fp, size, &load_sybmol, &ls);
  }
}

int
fndsym (const char *name)
{
  FILE *libfp;
  struct lbname *lbnh;
  int i;

  /*
   * Search through every library in the linked list "lbnhead".
   */

  D ("Searching symbol: %s\n", name);

  for (lbnh = lbnhead; lbnh; lbnh = lbnh->next)
    {
      int ret = 0;

      D ("Library: %s\n", lbnh->libspc);

      if ((libfp = fopen (lbnh->libspc, "rb")) == NULL)
        {
          fprintf (stderr, "?ASlink-Error-Cannot open library file %s\n", lbnh->libspc);
          lkexit (1);
        }

      for (i = 0; i < NELEM (aslib_targets); ++i)
        {
          if ((*aslib_targets[i]->is_lib) (libfp))
            {
              ret = (*aslib_targets[i]->fndsym) (name, lbnh, libfp, i);
              break;
            }
        }

      if (i >= NELEM (aslib_targets))
        fprintf (stderr, "?ASlink-Error-Unknown library file format %s\n", lbnh->libspc);

      fclose (libfp);

      if (ret)
        return 1;

    }                           /* Ends good open of libr file */
  return 0;
}
#endif /* INDEXLIB */

/*)Function VOID    library()
 *
 *  The function library() links all the library object files
 *  contained in the lbfile structures.
 *
 *  local variables:
 *      lbfile  *lbfh       pointer to lbfile structure
 *
 *  global variables:
 *      lbfile  *lbfhead    pointer to first lbfile structure
 *      int     obj_flag    linked file/library object output flag
 *
 *   functions called:
 *      VOID    loadfile    lklibr.c
 *
 *  side effects:
 *      Links all files contained in the lbfile structures.
 */

VOID
library (void)
{
  struct lbfile *lbfh;

  for (lbfh = lbfhead; lbfh; lbfh = lbfh->next) {
    obj_flag = lbfh->f_obj;
    (*aslib_targets[lbfh->type]->loadfile) (lbfh);
  }

#ifdef INDEXLIB
  freelibraryindex ();
#endif
}
