/* Copyright (C) 2002
 * This file is part of the ld86 command for Linux-86 
 * It is distributed under the GNU Library General Public License.
 *
 * CP/M-86 CMD file header
 */

#ifndef __CPM86_H
#define __CPM86_H

typedef char Short16[2];

struct  cpm86_group {
	unsigned char cg_type;	/* 1=Code 2=Data */
	Short16	      cg_len;	/* Group length, paragraphs */
	Short16	      cg_base;	/* Group address, normally 0 for relocatable */
	Short16	      cg_min;	/* Minimum size, normally = group length */
	Short16	      cg_max;	/* Maximum size, normally 0x1000 (64k) */
};


struct	cpm86_exec {			/* CP/M-86 header */
	struct cpm86_group ce_group[8];
	unsigned char ce_spare[51];
	Short16	      ce_rsxs;		/* Record with RSX list */
	Short16	      ce_fixups;	/* Record with fixups */
	unsigned char ce_flags;		/* Concurrent CP/M flags */
};

/* Group types */
#define CG_EMPTY 0
#define CG_CODE  1
#define CG_DATA  2
#define CG_EXTRA 3
#define CG_STACK 4
#define CG_AUX1  5
#define CG_AUX2  6
#define CG_AUX3  7
#define CG_AUX4  8
#define CG_PURE  9	/* Code that is known to be pure */

#define CPM86_HEADERLEN 0x80

#endif /* _CPM86_H */
