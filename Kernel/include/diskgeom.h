#ifndef _DISKGEOM_H
#define _DISKGEOM_H

/*
 *	The mini partition system used for legacy CHS environments or
 *	very small boxes.
 *
 *	The partitions are laid out in strict cylinder order. Each one ends
 *	at the start of the next with any spaces being a partition of type 0
 *	with start/end. The final partition and any beyond have mp_pcyl[n]
 *	set to mp_cyl.
 *
 *	mp_cyl is after the array so that it is always true that pcyl[n+1] is
 *	the first cylinder after partition n
 */
struct diskgeom {
  uint32_t magic;	/* Magic number */
#define MP_SIG_0	0x01CAD15CUL
  uint16_t cyl;		/* Cylinders on the drive. Must directly follow pcyl */
  uint8_t head;		/* Heads on the drive */
  uint8_t sec;		/* Sectors per track */
  uint16_t precomp;	/* Precompensation cylinder start (inclusive) */
#define PRECOMP_NONE	0xFFFF
  uint16_t land;	/* Usually cyl + 1 */
  uint8_t seek;		/* Seek step rate in ms */
  uint8_t secsize;	/* Sector size as power of 2 */
  uint8_t pad[2];	/* Takes us to 16 bytes */
};

/* Labelling bits needed for a CHS drive */
struct minipart {
  struct diskgeom g;
  uint16_t cyl[15];
  uint8_t type[15];
};

/* The full entry, including the LBA offsets added by the labeller
   for use if in LBA mode. The LBAs must match the cyl pointers */
struct minipart_lba {
  struct diskgeom g;
  uint16_t cyl[15];
  uint8_t type[15];
  uint8_t gap;
  uint32_t lba[15];
};

#endif
