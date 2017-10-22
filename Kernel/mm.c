#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Copy data to/from either kernel or user space into
 *	the buffer. This is needed because we do inode operations
 *	to read and write blocks both for userspace (normal disk I/O) and
 *	for the kernel (things like directory handling). For split I/D systems
 *	we will need to extend this three ways as we have to load code.
 *
 *	TODO: for the real block case add direct helpers for buf<->user
 *	transfer.
 */

unsigned int uputblk(bufptr bp, usize_t to, usize_t size)
{
	if (udata.u_sysio)
		blktok(udata.u_base, bp, to, size);
	else
		blktou(udata.u_base, bp, to, size);
	return size;
}

unsigned int ugetblk(bufptr bp, usize_t from, usize_t size)
{
	if (udata.u_sysio)
		blkfromk(udata.u_base, bp, from, size);
	else
		blkfromu(udata.u_base, bp, from, size);
	return size;
}
