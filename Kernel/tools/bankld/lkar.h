/* lkar.h - ar library format handling

   Copyright (C) 2008-2012 Borut Razem, borut dot razem at siol dot net

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

#ifndef __LKAR_H
#define __LKAR_H

#include <sys/types.h>
#include <string.h>


#ifdef _WIN32
typedef unsigned short mode_t;
typedef short uid_t;
typedef short gid_t;
typedef _off_t off_t;
#endif

#define sgetl(buf)  (((((((unsigned char)(buf)[0] << 8) + (unsigned char)(buf)[1]) << 8) + (unsigned char)(buf)[2]) << 8) + (unsigned char)(buf)[3])
#define sputl(value, buf)  ((buf)[0] = (value) >> 24, (buf)[1] = (value) >> 16, (buf)[2] = (value) >> 8, (buf)[3] = (value))

/* Note that the usual '\n' in magic strings may translate to different
   characters, as allowed by ANSI.  '\012' has a fixed value, and remains
   compatible with existing BSDish archives. */

#define ARMAG   "!<arch>\012"         /* magic string */
#define SARMAG  (sizeof (ARMAG) - 1)  /* length of magic string */

#define ARFMAG  "`\012"               /* header trailer string */

#define AR_NAME_OFFSET  0
#define AR_NAME_LEN     16

#define AR_DATE_OFFSET  16
#define AR_DATE_LEN     12

#define AR_UID_OFFSET   28
#define AR_UID_LEN      6

#define AR_GID_OFFSET   34
#define AR_GID_LEN      6

#define AR_MODE_OFFSET  40
#define AR_MODE_LEN     8

#define AR_SIZE_OFFSET  48
#define AR_SIZE_LEN     10

#define AR_FMAG_OFFSET  58
#define AR_FMAG_LEN     (sizeof (ARFMAG) - 1)

#define ARHDR_LEN (AR_NAME_LEN + AR_DATE_LEN + AR_UID_LEN + AR_GID_LEN + AR_MODE_LEN + AR_SIZE_LEN + AR_FMAG_LEN)

#define AR_SYMBOL_TABLE_NAME            "/               "
#define AR_STRING_TABLE_NAME            "//              "

#define AR_BSD_SYMBOL_TABLE_NAME        "__.SYMDEF       "
#define AR_BSD_SORTED_SYMBOL_TABLE_NAME "__.SYMDEF SORTED"

#define AR_IS_SYMBOL_TABLE(name) (0 == strcmp((name), AR_SYMBOL_TABLE_NAME))
#define AR_IS_STRING_TABLE(name) (0 == strcmp((name), AR_STRING_TABLE_NAME))

#define AR_IS_BSD_SYMBOL_TABLE(name) (0 == strcmp((name), AR_BSD_SYMBOL_TABLE_NAME) || 0 == strcmp((name), AR_BSD_SORTED_SYMBOL_TABLE_NAME))


struct ar_hdr                     /* archive member header */
{
  char ar_name[AR_NAME_LEN + 1];  /* archive member name */
  time_t ar_date;                 /* archive member date */
  uid_t ar_uid;                   /* archive member user identification */
  gid_t ar_gid;                   /* archive member group identification */
  mode_t ar_mode;                 /* archive member mode (octal) */
  size_t ar_size;                 /* archive member size */
};

#endif /* __LKAR_H */
