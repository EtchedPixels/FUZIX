#ifndef _SYS_SUPER_H
#define _SYS_SUPER_H

#include <sys/types.h>

typedef uint16_t blkno_t;
/*
 * Superblock structure
 */
#define FILESYS_TABSIZE 50
struct fuzix_filesys_kernel {
    uint16_t      s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    blkno_t       s_free[FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[FILESYS_TABSIZE];
    uint8_t       s_fmod;
#define FMOD_DIRTY	1	/* Mounted or uncleanly unmounted from r/w */
#define FMOD_CLEAN	2	/* Clean. Used internally to mean don't
				   update the super block */
    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
    uint32_t      s_time;
    blkno_t       s_tfree;
    uint16_t      s_tinode;
    uint8_t	  s_shift;	/* Extent size */
};

#define ROOTINODE 1
#define SMOUNTED 12742   /* Magic number to specify mounted filesystem */
#define SMOUNTED_WRONGENDIAN 50737U   /* byteflipped */

/*
 * Superblock with userspace fields that are not kept in the kernel
 * mount table.
 */
struct fuzix_filesys {
    struct fuzix_filesys_kernel s_fs;
    /* Allow for some kernel expansion */
    uint8_t	  s_reserved;
    uint16_t	  s_reserved2[16];
    /* This is only used by userspace */
    uint16_t	  s_props;	/* Property bits indicating which are valid */
#define S_PROP_LABEL	1
#define S_PROP_GEO	2
    /* For now only one property set - geometry. We'll eventually use this
       when we don't know physical geometry and need to handle stuff with
       tools etc */
    uint8_t	  s_label_name[32];

    uint16_t      s_geo_heads;	/* If 0/0/0 is specified and valid it means */
    uint16_t	  s_geo_cylinders; /* pure LBA - no idea of geometry */
    uint16_t	  s_geo_sectors;
    uint8_t	  s_geo_skew;	/* Soft skew if present (for hard sectored media) */
                                /* Gives the skew (1/2/3/4/5/... etc) */
    uint8_t	  s_geo_secsize;/* Physical sector size in log2 form */
};

#endif
