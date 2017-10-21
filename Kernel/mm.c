#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Copy data to/from either kernel or user space into
 *	the buffer. This is needed because we do inode operations
 *	to read and write blocks both for userspace (normal disk I/O) and
 *	for the kernel (things like directory handling).
 *
 *	TODO: for the real block case add direct helpers for buf<->user
 *	transfer.
 */
unsigned int uputblk(bufptr bp, usize_t to, usize_t size)
{
	if (udata.u_sysio)
		blktok(udata.u_base, bp, to, size);
	else
		uput(blkptr(bp, to, size), udata.u_base, size);
	return size;
}

unsigned int ugetblk(bufptr bp, usize_t from, usize_t size)
{
	if (udata.u_sysio)
		blkfromk(udata.u_base, bp, from, size);
	else
		uget(udata.u_base, blkptr(bp, from, size), size);
	return size;
}
