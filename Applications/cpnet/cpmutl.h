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

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__CC65__) || defined(__CC68__) || defined(__8085__)
#define __attribute__(x)
#endif

struct cpmfcb {
  uint8_t drive;        /* drive code */
  uint8_t name[8];      /* file name */
  uint8_t ext[3];       /* file type */
  uint8_t ex;           /* file extent */
  uint8_t s1, s2;
  uint8_t rc;           /* number of records in present extent */
  uint8_t dmap[16];     /* CP/M disk map */
  uint8_t cr;           /* next record to read or write */
  uint8_t r0, r1, r2;   /* random record number (16-bit value, r2 is ovfl) */
} __attribute__ ((packed));

struct cpmdpb {
  uint8_t  sptl;        /* sectors per track */
  uint8_t  spth;	      /* Z80 endian */
  uint8_t  bsh;         /* block shift */
  uint8_t  blm;         /* block mask */
  uint8_t  exm;         /* extent mask */
  uint8_t  dsml;        /* disk max (disk size - 1) */
  uint8_t  dsmh;
  uint8_t  drmh;        /* dir max */
  uint8_t  drml;
  uint8_t  al0, al1;    /* alloc 0, 1 */
  uint8_t  cksl;        /* check size */
  uint8_t  cksh;
  uint8_t  offh;         /* reserved tracks */
  uint8_t  offl;
} __attribute__ ((packed));


struct cpmfcb *get_dir_entry(DIR *dirp, struct cpmfcb *search_fcb, int first);
int delete_files(struct cpmfcb *fcb);
int update_allocv(void);
char *getname(struct cpmfcb *fcb);
int setname(struct cpmfcb *fcb, char *name);
int match_fcb(struct cpmfcb *mask, struct cpmfcb *fcb);


#endif  /* __cpmutl_h */
