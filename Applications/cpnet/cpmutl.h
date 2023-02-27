/**************************************************************************

    This file is part of the CP/NET 1.1/1.2 server emulator for Unix.
    Copyright (C) 2005, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#ifndef __cpmutl_h
#define __cpmutl_h

typedef unsigned short uint16;

#if defined(__SDCC_z80) || defined(__SDCC_z180)
#define __attribute__(x)
#endif

struct cpmfcb {
  uchar drive;        /* drive code */
  uchar name[8];      /* file name */
  uchar ext[3];       /* file type */
  uchar ex;           /* file extent */
  uchar s1, s2;
  uchar rc;           /* number of records in present extent */
  uchar dmap[16];     /* CP/M disk map */
  uchar cr;           /* next record to read or write */
  uchar r0, r1, r2;   /* random record number (16-bit value, r2 is ovfl) */
} __attribute__ ((packed));

struct cpmdpb {
  uchar  sptl;        /* sectors per track */
  uchar  spth;	      /* Z80 endian */
  uchar  bsh;         /* block shift */
  uchar  blm;         /* block mask */
  uchar  exm;         /* extent mask */
  uchar  dsml;        /* disk max (disk size - 1) */
  uchar  dsmh;
  uchar  drmh;        /* dir max */
  uchar  drml;
  uchar  al0, al1;    /* alloc 0, 1 */
  uchar  cksl;        /* check size */
  uchar  cksh;
  uchar  offh;         /* reserved tracks */
  uchar  offl;
} __attribute__ ((packed));


struct cpmfcb *get_dir_entry(DIR *dirp, struct cpmfcb *search_fcb, int first);
int delete_files(struct cpmfcb *fcb);
int update_allocv();
char *getname(struct cpmfcb *fcb);
int setname(struct cpmfcb *fcb, char *name);
int match_fcb(struct cpmfcb *mask, struct cpmfcb *fcb);


#endif  /* __cpmutl_h */
