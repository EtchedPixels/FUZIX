/*
 *	Atari partitions
 *
 *	BGM		: Big GEM
 *	GEM		: GEM
 *
 *	XGM		: Extended GEM (extended partition table)
 *
 */

struct atari_partition {
    uint8_t flags;
#define AP_ACTIVE	0x01
#define AP_BOOTABLE	0x80
    uint8_t id[3];
    uint32_t start;
    uint32_t length;
} __attribute((packed));

struct atari_bootblock {
    uint8_t bootcode[0x156];
    struct atari_partition icd[8];	/* ICD devices stick extra entries here */
    /* These are only used on very old devices and in fact it's not clear to me
       if any Atari STthing uses them! */
    uint16_t cylinders;
    uint8_t heads;
    uint8_t scsiflag;			/* FF for SCSI else SASI (but in fact
                                           it's ACSI! */
    uint16_t precomp;
    uint16_t rwcurrent;
    uint8_t park_offset;
    uint8_t step;
    uint8_t interleave;
    uint8_t spt;
    uint32_t size;			/* In blocks */
    struct atari_partition part[4];	/* Real partitions */

    uint32_t badsect;		/* We will need to deal with bad sectors */
    uint32_t badlen;		/* eventually... */
    uint16_t checksum;		/* _only_ if bootable */
} __attribute((packed));

