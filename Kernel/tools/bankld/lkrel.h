/* lkrel.h - .rel object file handling

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

#ifndef __LKREL_H
#define __LKREL_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  int is_rel (FILE * libfp);
  int load_rel (FILE * libfp, long size);
  int enum_symbols (FILE * fp, long size, int (*func) (const char *symvoid, void *param), void *param);


#ifdef __cplusplus
}
#endif

#endif                          /* __LKREL_H */
