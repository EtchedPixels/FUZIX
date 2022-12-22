#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

uaddr_t pagemap_base(void)
{
    return DATABASE;
}

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size)
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

void swap_blocks(void* p1, void* p2, size_t len)
{
    uint32_t* u1 = p1;
    uint32_t* u2 = p2;

    while (len != 0)
    {
        uint32_t t = *u1;
        *u1 = *u2;
        *u2 = t;
        u1++;
        u2++;
        len -= 4;
    }
}

/* vim: sw=4 ts=4 et: */


