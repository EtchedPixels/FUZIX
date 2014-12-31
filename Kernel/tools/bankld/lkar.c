/* lkar.c - ar library format handling

   Copyright (C) 1989-1995 Alan R. Baldwin
   721 Berkeley St., Kent, Ohio 44240
   Copyright (C) 2008-2009 Borut Razem, borut dot razem at gmail dot com

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

#include <assert.h>

#include "aslink.h"
#include "lklibr.h"
#include "lkrel.h"
#include "lkar.h"


#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static int
is_ar (FILE * libfp)
{
  char buf[SARMAG];
  int ret;

  if (!(ret = fread (buf, 1, sizeof (buf), libfp) == sizeof (buf) && memcmp (buf, ARMAG, SARMAG) == 0))
    rewind (libfp);

  return ret;
}

static char *str_tab;     /* string table */
static int str_tab_size;  /* string table size */

static char *
get_long_name (const char *name)
{
  assert ('/' == name[0]);

  if (NULL != str_tab)
    {
      char *p;

      int name_offset = strtol (++name, &p, 0);
      if (p != name && name_offset < str_tab_size)
        {
          int len = p - name + 1;
          while (len < AR_NAME_LEN && name[len++] == ' ')
            ;
          if (len == AR_NAME_LEN)
            {
              const char *n;

              /* long name: get it from the symbol table */
              name = &str_tab[name_offset];
              for (n = name; *n != '/' && *n != '\n'; ++n)
                assert (n < &str_tab[str_tab_size]);

              if (n[0] != '/' || n[1] != '\n')
                while (*++n != '\n')
                  assert (n < &str_tab[str_tab_size]);

              return strndup (name, n - name);
            }
        }
    }
  return NULL;
}

static char *
get_member_name (char *name, size_t *p_size, int allocate, FILE * libfp)
{
  if (p_size != NULL)
    *p_size = 0;

  if (0 == memcmp (name, "#1/", 3))
    {
      char *p;
      size_t len = strtoul (&name [3], &p, 10);
      if (p > &name [3])
        {
          /* BSD appends real file name to the file header */
          if (p_size != NULL)
            *p_size = len;

          if (allocate)
            {
              char *n = (char *) malloc (len);
              if (fread (n, 1, len, libfp) != len)
                {
                  /* not an ar archive or broken ar archive */
                  free (n);
                  return NULL;
                }
              else
                return n;
            }
          else
            {
              /* just advance the file pointer */
              fseek (libfp, len, SEEK_CUR);
              return NULL;
            }
        }
      else
        {
          /* not an ar archive or broken ar archive */
          return NULL;
        }
    }
  else if (allocate)
    {
      if (name[0] == '/')
        {
          char *n = get_long_name (name);
          if (NULL != n)
            return n;
        }
      else
        {
          const char *p = strrchr (name + 1, '/');

          if (NULL != p)
            {
              int len = p - name;
              while (name[++len] == ' ')
                ;
              if (len == AR_NAME_LEN)
                return strndup (name, p - name);
            }
          else
            {
              /* BSD formed member name:
                 trim trailing spaces */
              p = name + AR_NAME_LEN;
              while (*--p == ' ' && p >= name)
                ;
              return strndup (name, p - name + 1);
            }
        }

      /* bad formed member name or long name not found:
       just return it */

      return strdup (name);
    }
  else
    return NULL;
}

static size_t
ar_get_header (struct ar_hdr *hdr, FILE * libfp, char **p_obj_name)
{
  char header[ARHDR_LEN];
  char buf[AR_DATE_LEN + 1];
  char *obj_name;
  size_t size;

  if (fread (header, 1, sizeof (header), libfp) != sizeof (header)
      || memcmp (header + AR_FMAG_OFFSET, ARFMAG, AR_FMAG_LEN) != 0)
    {
      /* not an ar archive */
      return 0;
    }

  memcpy (hdr->ar_name, &header[AR_NAME_OFFSET], AR_NAME_LEN);
  hdr->ar_name[AR_NAME_LEN] = '\0';

  memcpy (buf, &header[AR_DATE_OFFSET], AR_DATE_LEN);
  buf[AR_DATE_LEN] = '\0';
  hdr->ar_date = strtol (buf, NULL, 0);

  memcpy (buf, &header[AR_UID_OFFSET], AR_GID_LEN);
  buf[AR_GID_LEN] = '\0';
  hdr->ar_uid = (uid_t) strtol (buf, NULL, 0);

  memcpy (buf, &header[AR_GID_OFFSET], AR_DATE_LEN);
  buf[AR_DATE_LEN] = '\0';
  hdr->ar_gid = (gid_t) strtol (buf, NULL, 0);

  memcpy (buf, &header[AR_MODE_OFFSET], AR_MODE_LEN);
  buf[AR_MODE_LEN] = '\0';
  hdr->ar_mode = (mode_t) strtoul (buf, NULL, 0);

  memcpy (buf, &header[AR_SIZE_OFFSET], AR_SIZE_LEN);
  buf[AR_SIZE_LEN] = '\0';
  hdr->ar_size = strtol (buf, NULL, 0);

  obj_name = get_member_name (hdr->ar_name, &size, p_obj_name != NULL, libfp);

  if (p_obj_name != NULL)
    *p_obj_name = obj_name;

  /* treat BSD appended real file name as a part of the header */
  hdr->ar_size -= size;

  return size + ARHDR_LEN;
}

#ifdef INDEXLIB
static char *
get_member_name_by_offset (FILE * fp, long offset)
{
  struct ar_hdr hdr;
  char *name;

  fseek (fp, offset, SEEK_SET);
  return (ar_get_header (&hdr, fp, &name) != 0) ? name : NULL;
}

static pmlibraryfile
find_member_by_offset (const char *libspc, long offset)
{
  pmlibraryfile p;

  /* walk trough all archive members */
  for (p = libr; p; p = p->next)
    {
      if (0 == strcmp (libspc, p->libspc) && p->offset == offset)
        return p;
    }

  return NULL;
}

static pmlibraryfile
buildlibraryindex_ar (struct lbname *lbnh, FILE * libfp, pmlibraryfile This, int type)
{
  struct ar_hdr hdr;
  char *obj_name;
  size_t hdr_size;
  int sym_found = 0;

  /* walk trough all archive members */
  while ((hdr_size = ar_get_header (&hdr, libfp, &obj_name)) != 0)
    {
      long pos = ftell (libfp);

      if (AR_IS_SYMBOL_TABLE (obj_name))
        {
          char *buf, *po, *ps;
          int i;
          long nsym;

          /* duplicated symbol table */
          assert (!sym_found);

          free (obj_name);

          buf = (char *) new (hdr.ar_size);

          if (fread (buf, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (buf);
              return This;
            }

          nsym = sgetl (buf);

          po = buf + 4;
          ps = po + nsym * 4;

          for (i = 0; i < nsym; ++i)
            {
              pmlibrarysymbol ThisSym;
              char *sym;
              long offset;
              pmlibraryfile entry;

              offset = sgetl (po);
              po += 4;

              sym = strdup (ps);
              ps += strlen (ps) + 1;

              if ((entry = find_member_by_offset (lbnh->libspc, offset)) != NULL)
                {
                  for (ThisSym = entry->symbols; ThisSym->next != NULL; ThisSym = ThisSym->next)
                    ;
                }
              else
                {
                  /* Opened OK - create a new libraryfile object for it */
                  if (This == NULL)
                    {
                      assert (libr == NULL);
                      libr = This = (pmlibraryfile) new (sizeof (mlibraryfile));
                    }
                  else
                    {
                      This->next = (pmlibraryfile) new (sizeof (mlibraryfile));
                      This = This->next;
                    }
                  This->next = NULL;
                  This->loaded = 0;
                  This->libspc = lbnh->libspc;
                  This->offset = offset;
                  This->relfil = get_member_name_by_offset (libfp, offset);     /* member name */
                  This->filspc = strdup (This->relfil); /* member file name */
                  This->type = type;

                  /* start a new linked list of symbols for this module. */
                  This->symbols = ThisSym = NULL;
                }

              if (ThisSym == NULL)
                ThisSym = This->symbols = (pmlibrarysymbol) new (sizeof (mlibrarysymbol));
              else
                {
                  ThisSym->next = (pmlibrarysymbol) new (sizeof (mlibrarysymbol));
                  ThisSym = ThisSym->next;
                }
              ThisSym->next = NULL;
              ThisSym->name = sym;
            }
          free (buf);

          sym_found = 1;

          /* string table already found: finish */
          if (str_tab)
            break;
        }
      else if (AR_IS_BSD_SYMBOL_TABLE (obj_name))
        {
          char *buf, *po, *ps;
          int i;
          long nsym, tablesize;

          /* duplicated symbol table */
          assert (!sym_found);

          free (obj_name);

          buf = (char *) new (hdr.ar_size);

          if (fread (buf, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (buf);
              return This;
            }

          tablesize = sgetl (buf);
          nsym = tablesize / 8;

          po = buf + 4;

          ps = po + tablesize + 4;

          for (i = 0; i < nsym; ++i)
            {
              pmlibrarysymbol ThisSym;
              char *sym;
              long offset;
              pmlibraryfile entry;

              sym = ps + sgetl (po);
              po += 4;
              offset = sgetl (po);
              po += 4;

              sym = strdup (ps);

              if ((entry = find_member_by_offset (lbnh->libspc, offset)) != NULL)
                {
                  for (ThisSym = entry->symbols; ThisSym->next != NULL; ThisSym = ThisSym->next)
                    ;
                }
              else
                {
                  /* Opened OK - create a new libraryfile object for it */
                  if (This == NULL)
                    {
                      assert (libr == NULL);
                      libr = This = (pmlibraryfile) new (sizeof (mlibraryfile));
                    }
                  else
                    {
                      This->next = (pmlibraryfile) new (sizeof (mlibraryfile));
                      This = This->next;
                    }
                  This->next = NULL;
                  This->loaded = 0;
                  This->libspc = lbnh->libspc;
                  This->offset = offset;
                  This->relfil = get_member_name_by_offset (libfp, offset);     /* member name */
                  This->filspc = strdup (This->relfil); /* member file name */
                  This->type = type;

                  /* start a new linked list of symbols for this module. */
                  This->symbols = ThisSym = NULL;
                }

              if (ThisSym == NULL)
                ThisSym = This->symbols = (pmlibrarysymbol) new (sizeof (mlibrarysymbol));
              else
                {
                  ThisSym->next = (pmlibrarysymbol) new (sizeof (mlibrarysymbol));
                  ThisSym = ThisSym->next;
                }
              ThisSym->next = NULL;
              ThisSym->name = sym;
            }
          free (buf);

          sym_found = 1;

          /* string table already found: finish */
          if (str_tab)
            break;
        }
      else if (AR_IS_STRING_TABLE (obj_name))
        {
          free (obj_name);

          /* duplicated string table */
          assert (NULL == str_tab);

          str_tab = (char *) new (hdr.ar_size);

          if (fread (str_tab, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (str_tab);
              str_tab_size = 0;
              return This;
            }
          str_tab_size = hdr.ar_size;

          /* sybol table already found: finish */
          if (sym_found)
            break;
        }
      else
        {
          if (NULL == libr)
            {
              /* Opened OK - create a new libraryfile object for it */
              if (This == NULL)
                {
                  assert (libr == NULL);
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
              This->offset = pos - hdr_size;

              This->relfil = obj_name;              /* member name */
              This->filspc = strdup (This->relfil); /* member file name */

              D ("  Indexing module: %s\n", This->relfil);

              This->type = type;

              /* start a new linked list of symbols for this module. */
              This->symbols = NULL;

              add_rel_index (libfp, hdr.ar_size, This);
            }
        }
      fseek (libfp, pos + hdr.ar_size + (hdr.ar_size & 1), SEEK_SET);
    }

  if (NULL != str_tab)
    {
      /* has a symbol table: walk through modules and replace offsets with names */
      pmlibraryfile lfp;

      for (lfp = libr; lfp; lfp = lfp->next)
        {
          char *name = lfp->relfil;
          if (name[0] == '/')
            {
              char *p = get_long_name (name);
              if (NULL != p)
                {
                  free (lfp->relfil);
                  lfp->relfil = p;
                  free (lfp->filspc);
                  lfp->filspc = strdup (p);
                }
            }
        }


      free (str_tab);
      str_tab = NULL;
      str_tab_size = 0;
    }

  return This;
}

#else

#if 0
static int
load_adb (FILE * libfp, struct lbfile *lbfh)
{
  struct ar_hdr hdr;
  char *adb_name;
  char *obj_name;
  size_t hdr_size;

  /* check if it is a .rel file */
  if (0 != stricmp (&lbfh->relfil[strlen (lbfh->relfil) - 4], ".rel"))
    return 0;


  adb_name = (char *) new (strlen (lbfh->relfil) + 1);
  memcpy (adb_name, lbfh->relfil, strlen (lbfh->relfil) - 4);
  memcpy (&adb_name[strlen (lbfh->relfil) - 4], ".adb", 5);

  if (!is_ar (libfp))
    {
      fprintf (stderr, "?ASlink-Error-%s is not an archive\n", lbfh->libspc);
      fclose (libfp);
      lkexit (1);
    }

  /* walk trough all archive members */
  while ((hdr_size = ar_get_header (&hdr, libfp, &obj_name)) != 0)
    {
      if (AR_IS_STRING_TABLE (obj_name))
        {
          free (obj_name);

          if (str_tab)
            free (str_tab);

          str_tab = (char *) new (hdr.ar_size);

          if ((off_t) fread (str_tab, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (str_tab);
              str_tab_size = 0;
              return 0;
            }
          str_tab_size = hdr.ar_size;
        }
      if (AR_IS_SYMBOL_TABLE (obj_name) || 0 != stricmp (obj_name, adb_name))
        {
          free (obj_name);

          /* skip the mamber */
          fseek (libfp, hdr.ar_size + (hdr.ar_size & 1), SEEK_CUR);
        }
      else
        {
          long left = hdr.ar_size;
          char buf[4096];

          free (obj_name);

          while (left)
            {
              size_t n = min (left, sizeof buf);

              if (fread (buf, 1, n, libfp) != n)
                {
                  assert (0);
                }

              fwrite (buf, 1, n, yfp);

              left -= n;
            }

          if (hdr.ar_size & 1)
            getc (libfp);

          free (adb_name);
          return 1;
        }
    }

  free (adb_name);
  return 0;
}
#endif

static void
load_str_tab (FILE * libfp)
{
  struct ar_hdr hdr;
  char *obj_name;
  size_t hdr_size;

  /* walk trough all archive members */
  while ((hdr_size = ar_get_header (&hdr, libfp, &obj_name)) != 0)
    {
      long pos = ftell (libfp);

      if (AR_IS_STRING_TABLE (obj_name))
        {
          free (obj_name);

          /* duplicated string table */
          assert (NULL == str_tab);

          str_tab = (char *) new (hdr.ar_size);

          if (fread (str_tab, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (str_tab);
              str_tab_size = 0;
              return;
            }
          str_tab_size = hdr.ar_size;
          return;

        }
      fseek (libfp, pos + hdr.ar_size + (hdr.ar_size & 1), SEEK_SET);
    }
}

static int
fndsym_ar (const char *name, struct lbname *lbnh, FILE * libfp, int type)
{
  struct ar_hdr hdr;
  int ret = 0;
  size_t hdr_size;
  char *obj_name;
  long pos;

  pos = ftell (libfp);
  load_str_tab (libfp);
  fseek (libfp, pos, SEEK_SET);

  /* walk trough all archive members */
  while ((hdr_size = ar_get_header (&hdr, libfp, &obj_name)) != 0)
    {
      char filspc[PATH_MAX] = { 0 };

      if (lbnh->path != NULL)
        {
          strcpy (filspc, lbnh->path);
          if (*filspc != '\0' && (filspc[strlen (filspc) - 1] != '/') && (filspc[strlen (filspc) - 1] != LKDIRSEP))
            {
              strcat (filspc, LKDIRSEPSTR);
            }
        }

      if (AR_IS_SYMBOL_TABLE (obj_name))
        {
          char *buf, *po, *ps;
          int i;
          long nsym;

          free (obj_name);

          buf = (char *) new (hdr.ar_size);

          if (fread (buf, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (buf);
              return 0;
            }

          nsym = sgetl (buf);

          po = buf + 4;
          ps = po + nsym * 4;

          for (i = 0; i < nsym; ++i)
            {
              char *sym;
              long offset;

              offset = sgetl (po);
              po += 4;

              sym = ps;
              while (*ps++ != '\0')
                ;

              if (0 == strcmp (name, sym))
                {
                  fseek (libfp, offset, SEEK_SET);
                  if (ar_get_header (&hdr, libfp, &obj_name))
                    {
                      sprintf (&filspc[strlen (filspc)], "%s", hdr.ar_name);

                      /* If this module has been loaded already don't load it again. */
                      if (!is_module_loaded (filspc))
                        {
                          struct lbfile *lbfh, *lbf;

                          lbfh = (struct lbfile *) new (sizeof (struct lbfile));
                          lbfh->libspc = strdup (lbnh->libspc);
                          lbfh->relfil = obj_name;
                          lbfh->filspc = strdup (filspc);
                          lbfh->offset = offset;
                          lbfh->type = type;

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

                          D ("Loading module %s from file %s.\n", hdr.ar_name, lbfh->libspc);
                          load_rel (libfp, hdr.ar_size);
                          ///* if cdb information required & .adb file present */
                          //if (yflag && yfp)
                          //  {
                          //    if (load_adb(FILE *libfp, struct lbfile *lbfh))
                          //      SaveLinkedFilePath (filspc);
                          //  }
                          ret = 1;
                          break;
                        }
                    }
                  else
                    {
                      fprintf (stderr, "?ASlink-Error-Bad offset in library file %s(%s)\n", lbnh->libspc, name);
                      fclose (libfp);
                      lkexit (1);
                    }
                }
            }
          free (buf);

          break;
        }
      else if (AR_IS_BSD_SYMBOL_TABLE (obj_name))
        {
          char *buf, *po, *ps;
          int i;
          long nsym, tablesize;

          free (obj_name);

          buf = (char *) new (hdr.ar_size);

          if (fread (buf, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (buf);
              return 0;
            }

          tablesize = sgetl (buf);
          nsym = tablesize / 8;

          po = buf + 4;

          ps = po + tablesize + 4;

          for (i = 0; i < nsym; ++i)
            {
              char *sym;
              long offset;

              sym = ps + sgetl (po);
              po += 4;
              offset = sgetl (po);
              po += 4;

              if (0 == strcmp (name, sym))
                {
                  fseek (libfp, offset, SEEK_SET);
                  if (ar_get_header (&hdr, libfp, &obj_name))
                    {
                      sprintf (&filspc[strlen (filspc)], "%s", hdr.ar_name);

                      /* If this module has been loaded already don't load it again. */
                      if (!is_module_loaded (filspc))
                        {
                          struct lbfile *lbfh, *lbf;

                          lbfh = (struct lbfile *) new (sizeof (struct lbfile));
                          lbfh->libspc = strdup (lbnh->libspc);
                          lbfh->relfil = obj_name;
                          lbfh->filspc = strdup (filspc);
                          lbfh->offset = offset;
                          lbfh->type = type;

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

                          D ("Loading module %s from file %s.\n", hdr.ar_name, lbfh->libspc);
                          load_rel (libfp, hdr.ar_size);
                          ///* if cdb information required & .adb file present */
                          //if (yflag && yfp)
                          //  {
                          //    if (load_adb(FILE *libfp, struct lbfile *lbfh))
                          //      SaveLinkedFilePath (filspc);
                          //  }
                          ret = 1;
                          break;
                        }
                    }
                  else
                    {
                      fprintf (stderr, "?ASlink-Error-Bad offset in library file %s(%s)\n", lbnh->libspc, name);
                      fclose (libfp);
                      lkexit (1);
                    }
                }
            }
          free (buf);

          break;
        }
      else if (AR_IS_STRING_TABLE (obj_name))
        {
          free (obj_name);

          if (str_tab)
            free (str_tab);

          str_tab = (char *) new (hdr.ar_size);

          if (fread (str_tab, 1, hdr.ar_size, libfp) != hdr.ar_size)
            {
              free (str_tab);
              str_tab = NULL;
              str_tab_size = 0;
              return 0;
            }
          str_tab_size = hdr.ar_size;
        }
      else
        {
          long pos = ftell (libfp);

          free (obj_name);

          D ("  Module: %s\n", hdr.ar_name);

          sprintf (&filspc[strlen (filspc)], "%s", hdr.ar_name);

          /* Opened OK - create a new libraryfile object for it */
          ret = add_rel_file (name, lbnh, hdr.ar_name, filspc, pos - hdr_size, libfp, hdr.ar_size, type);
          ///* if cdb information required & .adb file present */
          //if (yflag && yfp)
          //  {
          //    if (load_adb(FILE *libfp, struct lbfile *lbfh))
          //      SaveLinkedFilePath (filspc);
          //  }
          if (ret)
            break;

          fseek (libfp, pos + hdr.ar_size + (hdr.ar_size & 1), SEEK_SET);
        }
    }

  if (NULL != str_tab)
    {
      free (str_tab);
      str_tab = NULL;
      str_tab_size = 0;
    }

  return ret;
}
#endif

static void
loadfile_ar (struct lbfile *lbfh)
{
  FILE *fp;

#ifdef __CYGWIN__
  char posix_path[PATH_MAX];
  void cygwin_conv_to_full_posix_path (char *win_path, char *posix_path);
  cygwin_conv_to_full_posix_path (lbfh->libspc, posix_path);
  fp = fopen (posix_path, "rb");
#else
  fp = fopen (lbfh->libspc, "rb");
#endif

  if (fp != NULL)
    {
      struct ar_hdr hdr;

      fseek (fp, lbfh->offset, SEEK_SET);
      if (ar_get_header (&hdr, fp, NULL) != 0)
        {
          D ("Loading module %s from file %s.\n", hdr.ar_name, lbfh->libspc);
          load_rel (fp, hdr.ar_size);
          fclose (fp);
        }
      else
        {
          fprintf (stderr, "?ASlink-Error-Bad offset in library file %s(%s)\n", lbfh->libspc, lbfh->relfil);
          fclose (fp);
          lkexit (1);
        }
    }
  else
    {
      fprintf (stderr, "?ASlink-Error-Opening library '%s'\n", lbfh->libspc);
      lkexit (1);
    }
}

struct aslib_target aslib_target_ar = {
  &is_ar,
#ifdef INDEXLIB
  &buildlibraryindex_ar,
#else
  &fndsym_ar,
#endif
  &loadfile_ar,
};
