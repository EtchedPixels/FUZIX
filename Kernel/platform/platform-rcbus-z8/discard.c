#include <kernel.h>

/* Add memory */
void pagemap_init(void)
{
	/* Low 0 is kernel map, high 0 unused except at boot */
	/* 0xC3 is the kernel data */
	/* pagemap_add(0xC3); */
	pagemap_add(0x33);
	pagemap_add(0xF3);
	pagemap_add(0x0F);
	pagemap_add(0xCF);
	pagemap_add(0x3F);
	/* This one weill be picked up by init and must be first to
	   match crt0.S */
	pagemap_add(0xFF);
}

void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
 used(p);
 return 0;
}
