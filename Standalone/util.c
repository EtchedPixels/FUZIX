#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fuzix_fs.h"
#include "util.h"

int dev_fd;
int dev_offset;
int swapped;
int swizzling;

extern int swizzling;

int fd_open(char *name, int addflags)
{
	if (bdopen(name, addflags) < 0)
		exit(1);
	/* printf("fd=%d, dev_offset = %d\n", dev_fd, dev_offset); */
	return 0;
}

void fd_close(void)
{
	bdclose();
}

void panic(char *s)
{
	fprintf(stderr, "panic: %s\n", s);
	exit(1);
}

uint16_t swizzle16(uint32_t v)
{
        int top = v & 0xFFFF0000UL;
	if (top && top != 0xFFFF0000) {
		fprintf(stderr, "swizzle16 given a 32bit input\n");
		exit(1);
	}
	if (swizzling)
		return (v & 0xFF) << 8 | ((v & 0xFF00) >> 8);
	else
		return v;
}

uint32_t swizzle32(uint32_t v)
{
	if (!swizzling)
		return v;

	return (v & 0xFF) << 24 | (v & 0xFF00) << 8 | (v & 0xFF0000) >> 8 |
	    (v & 0xFF000000) >> 24;
}

/*********************************************************************
Bdread() and bdwrite() are the block device interface routines.  they
are given a buffer pointer, which contains the device, block number,
and data location.  They basically validate the device and vector the
call.

Cdread() and cdwrite are the same for character (or "raw") devices,
and are handed a device number.  Udata.u_base, count, and offset have
the rest of the data.
**********************************************************************/

static uint8_t *bdswap(uint8_t * p)
{
	static uint8_t buf[512];
	if (!swapped)
		return p;
	swab(p, buf, 512);
	return buf;
}

static void bdswapkeep(uint8_t * p)
{
	/* Irritatingly POSIX says swab(x,x,512) is undefined even though
	   it's actually pretty safe */
	memcpy(p, bdswap(p), 512);
}


#ifdef LIBDSK
#include <libdsk.h>

static DSK_PDRIVER drive;
static DSK_GEOMETRY dg;
static int libdsk;

static int blkshift(unsigned int *blk)
{
	switch(dg.dg_secsize) {
	case 128:
		*blk <<= 2;
		return 4;
	case 256:
		*blk <<= 1;
		return 2;
	case 512:
		return 1;
	default:
		fprintf(stderr, "ucp block size %ld not supported.\n",
			dg.dg_secsize);
		exit(1);
	}
}

static int bdread_libdsk(unsigned int blk, uint8_t *dbase)
{
	uint8_t *dp = dbase;
	unsigned int n = blkshift(&blk);

	while(n--) {
		if (dsk_lread(drive, &dg, dp, blk++) < 0)
			return -1;
		dp += dg.dg_secsize;
	}
	if (swapped)
		bdswapkeep(dbase);
	return 0;
}

static int bdwrite_libdsk(unsigned int blk, uint8_t *dp)
{
	unsigned int n = blkshift(&blk);
	dp = bdswap(dp);

	while(n--) {
		if (dsk_lwrite(drive, &dg, dp, blk++) < 0)
			return -1;
		dp += dg.dg_secsize;
	}
	return 0;
}

static int bdopen_libdsk(const char *name, int addflags)
{
	int err;

	err = dsk_open(&drive, name, NULL, NULL);
	if (err) {
		fprintf(stderr, "Unable to open '%s' with libdsk.\n", name);
		return -1;
	}
	err = dsk_getgeom(drive, &dg);
	if (err)
		return -1;
	libdsk = 1;
	printf("Opening '%s' with %ld byte sectors using libdsk.\n", name,
		dg.dg_secsize);
	return 0;
}

static void bdclose_libdsk(void)
{
	dsk_close(&drive);
}

#endif

const uint8_t ide_magic[8] = {
	'1','D','E','D','1','5','C','0'
};

static int bdread_raw(unsigned int blk, uint8_t *dp)
{
	if (lseek(dev_fd, dev_offset + blk * 512,
		  SEEK_SET) == -1)
		perror("lseek");
	if (read(dev_fd, dp, 512) != 512)
		panic("read() failed");
	if (swapped)
		bdswapkeep(dp);
	return 0;
}

static int bdwrite_raw(unsigned int blk, uint8_t *dp)
{
	lseek(dev_fd, dev_offset + blk * 512, SEEK_SET);
	if (write(dev_fd, bdswap(dp), 512) != 512)
		panic("write() failed");
	return 0;
}

static int bdopen_raw(const char *name, int addflags)
{
	uint8_t tmp[512];
	char *namecopy = strdup(name);
	char *sd = strrchr(namecopy, ':');
	if (sd) {
		*sd = 0;
		sd++;
		dev_offset = atoi(sd);
	}

	printf("Opening %s (offset %d)\n", namecopy, dev_offset);

	dev_fd = open(namecopy, O_RDWR|addflags, 0600);
	if (dev_fd == -1) {
		perror(namecopy);
		free(namecopy);
		return -1;
	}
	free(namecopy);
	if (read(dev_fd, tmp, 512) != 512) {
		/* Creating a volume so don't require a size */
		if (addflags & O_CREAT)
			return dev_fd;
		fputs("Unable to read 512 bytes of data.\n", stderr);
		exit(1);
	}
	if (memcmp(tmp, ide_magic, 8) == 0) {
		puts("Volume is virtual IDE drive.");
		dev_offset += 1024;
		return 0;
	}
	if (memcmp(tmp, "RS-IDE", 6) == 0) {
		puts("Volume is HDF.");
		dev_offset += 534;
	}
	return dev_fd;
}

static void bdclose_raw(void)
{
	if (close(dev_fd))
		perror("close");
}

int bdread(unsigned int blk, uint8_t *dp)
{
#ifdef LIBDSK
	if (libdsk)
		return bdread_libdsk(blk, dp);
#endif
	return bdread_raw(blk, dp);
}


int bdwrite(unsigned int blk, uint8_t *dp)
{
#ifdef LIBDSK
	if (libdsk)
		return bdwrite_libdsk(blk, dp);
#endif
	return bdwrite_raw(blk, dp);
}

void bdclose(void)
{
#ifdef LIBDSK
	if (libdsk)
		bdclose_libdsk();
	else
#endif
	bdclose_raw();
}

int bdopen(const char *name, int addflags)
{
#ifdef LIBDSK
	if (strncmp(name, "libdsk:", 7) == 0) {
		printf("MOO");
		return bdopen_libdsk(name + 7, addflags);
	} else
#endif
	return bdopen_raw(name, addflags);
}
