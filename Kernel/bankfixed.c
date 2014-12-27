#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_BANK_FIXED

/* Kernel is 0, apps 1,2,3 etc */
static unsigned char pfree[MAX_MAPS];
static unsigned char pfptr = 0;
static unsigned char pfmax;

void pagemap_add(uint8_t page)
{
	pfree[pfptr++] = page;
	pfmax = pfptr;
}

void pagemap_free(ptptr p)
{
	if (p->p_page == 0)
		panic("free0");
	pfree[pfptr++] = p->p_page;
}

int pagemap_alloc(ptptr p)
{
#ifdef SWAPDEV
	if (pfptr == 0) {
		swapneeded(p, 1);
	}
#endif
	if (pfptr == 0)
		return ENOMEM;
	p->p_page = pfree[--pfptr];
	return 0;
}

/* Realloc is trivial - we can't do anything useful */
int pagemap_realloc(uint16_t size)
{
	if (size >= MAP_SIZE)
		return ENOMEM;
	return 0;
}

uint16_t pagemap_mem_used(void)
{
	return (pfmax - pfptr) * (MAP_SIZE >> 10);
}

#endif
