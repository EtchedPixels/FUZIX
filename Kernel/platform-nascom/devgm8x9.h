#ifndef __DEVFD_DOT_H__
#define __DEVFD_DOT_H__

/* public interface */
int gm8x9_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int gm8x9_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int gm8x9_open(uint8_t minor, uint16_t flag);

/* low level interface */
uint8_t gm8x9_seek(uint8_t track) __z88dk_fastcall;
uint8_t gm8x9_restore(void);
uint8_t gm8x9_restore_test(void);
uint8_t gm8x9_reset(void);
uint8_t gm8x9_ioread(uint8_t *dptr) __z88dk_fastcall;
uint8_t gm8x9_iowrite(uint8_t *dptr) __z88dk_fastcall;

#define MAX_GMFD		16
#define MAX_SKEW		32

struct gmfd {
	uint8_t track;		/* Saved track value */
	uint8_t spt;		/* Sectors per track (including both sides if DS) */
	uint8_t bs;		/* Block shift to sectors */
	uint16_t ss;		/* Sector size */
	uint8_t ds;		/* Sector that starts second side, 255 = SS */
	uint8_t dens;		/* Density and sides info, inc 8 v 5.25 */
	uint8_t skewtab[MAX_SKEW];	/* Skew table */
	uint8_t steprate;
};

extern struct gmfd gmfd_drives[MAX_GMFD];


#endif /* __DEVFD_DOT_H__ */
