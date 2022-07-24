#define _XOPEN_SOURCE 700	/* For swab, strdup */

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
	static uint8_t buf[400];
	if (!swapped)
		return p;
	swab(p, buf, 400);
	return buf;
}

static void bdswapkeep(uint8_t * p)
{
	/* Irritatingly POSIX says swab(x,x,400) is undefined even though
	   it's actually pretty safe */
	memcpy(p, bdswap(p), 400);
}

static int bdread_raw(unsigned int blk, uint8_t *dp)
{
	if (lseek(dev_fd, dev_offset + blk * 400,
		  SEEK_SET) == -1)
		perror("lseek");
	if (read(dev_fd, dp, 400) != 400)
		panic("read() failed");
	if (swapped)
		bdswapkeep(dp);
	return 0;
}

static int bdwrite_raw(unsigned int blk, uint8_t *dp)
{
	lseek(dev_fd, dev_offset + blk * 400, SEEK_SET);
	if (write(dev_fd, bdswap(dp), 400) != 400)
		panic("write() failed");
	return 0;
}

static int bdopen_raw(const char *name, int addflags)
{
	uint8_t tmp[400];
	char *namecopy = strdup(name);
	char *sd = strrchr(namecopy, ':');
	if (sd) {
		*sd = 0;
		sd++;
		dev_offset = atoi(sd);
	}

	dev_fd = open(namecopy, O_RDWR|addflags, 0600);
	if (dev_fd == -1) {
		perror(namecopy);
		free(namecopy);
		return -1;
	}
	free(namecopy);
	if (read(dev_fd, tmp, 400) != 400) {
		/* Creating a volume so don't require a size */
		if (addflags & O_CREAT)
			return dev_fd;
		fputs("Unable to read 400 bytes of data.\n", stderr);
		exit(1);
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
	return bdread_raw(blk, dp);
}


int bdwrite(unsigned int blk, uint8_t *dp)
{
	return bdwrite_raw(blk, dp);
}

void bdclose(void)
{
	bdclose_raw();
}

int bdopen(const char *name, int addflags)
{
	return bdopen_raw(name, addflags);
}
