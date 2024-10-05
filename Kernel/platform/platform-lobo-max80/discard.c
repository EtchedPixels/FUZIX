#include <kernel.h>
#include <printf.h>
#include <tinyscsi.h>
#include <devtty.h>
#include <tty.h>

void device_init(void)
{
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  scsi_probe(7);
}

void map_init(void)
{
}

void pagemap_init(void)
{
 pagemap_add(0x80);	/* Chunk 1 A low, chunk 1 B high */
 pagemap_add(0xC0);	/* Chunk 1 A low, chunk 2 B high */
}
