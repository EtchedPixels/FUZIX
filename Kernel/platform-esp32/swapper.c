#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

#undef DEBUG

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size, uint_fast8_t is_write)
{
	if (base + size < base)
		size = MAXUSIZE - (usize_t)base + 1;
	if (!base || base < (const uint8_t *)DATABASE)
		size = 0;
	else if (base + size > (const uint8_t *)(size_t)udata.u_top)
		size = (uint8_t *)(size_t)udata.u_top - base;
	if (size == 0)
		udata.u_error = EFAULT;
	return size;
}

usize_t valaddr_r(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 0);
}

usize_t valaddr_w(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 1);
}

uaddr_t pagemap_base(void)
{
	return DATABASE;
}

void pagemap_free(ptptr p)
{
	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	p->p_page = 1;
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return (DATALEN + CODELEN) >> 10;
}

void pagemap_init(void)
{
}

static void swapinout(int blk, void* u,
	int (*readwrite)(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    uaddr_t buf, uint16_t page))
{
    panic("cannot swap");
}

/*
 *	Swap out the memory of a process to make room
 *	for something else
 */
int swapout_new(ptptr p, void *u)
{
    panic("cannot swap");
	return 0;
}

int swapout(ptptr p)
{
	return swapout_new(p, (uint8_t*)&udata);
}

/*
 * Swap ourself in: must be on the swap stack when we do this
 */
void swapin(ptptr p, uint16_t map)
{
    panic("cannot swap");
}

/* vim: sw=4 ts=4 et: */

