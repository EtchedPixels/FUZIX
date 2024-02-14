#include <kernel.h>

/* Add memory */
void pagemap_init(void)
{
    register unsigned i;
    /* Simple banking to get us going */
    for (i = 35; i <= 63; i+=3)
        pagemap_add(i);
    /* TODO: do we need 35 last ? */
}

void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
 used(p);
 return 0;
}
