/*
 *	A minimal implementation of MBR parsing for small systems. Only looks
 *	for primary partitions but does deal with swap finding.
 *
 *	TODO: way to call so that tinydisk_setup is told "but not swap")
 *	TODO: allow module to be compiled in not discard and handle media change
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#define _TINYDISK_PRIVATE
#include <tinydisk.h>
#include "mbr.h"

#ifdef CONFIG_DYNAMIC_SWAP

static void swap_found(uint_fast8_t minor, partition_table_entry_t * pe)
{
	uint32_t off;
	uint16_t n = 0;
	if (swap_dev != 0xFFFF)
		return;
	kputs("(swap) ");
	swap_dev = minor;	/* major is 0 */
	off = le32_to_cpu(pe->lba_count);

	while (off > SWAP_SIZE && n < MAX_SWAPS) {
		off -= SWAP_SIZE;
		n++;
	}
	while (n)
		swapmap_init(--n);
}
#endif

#ifdef CONFIG_DYNAMIC_PAGE
#include <page.h>

static void swap_found(uint_fast8_t minor, partition_table_entry_t * pe)
{
	uint32_t off;

	if (swap_dev != 0xFFFF)
		return;
	kputs("(page) ");
	swap_dev = minor;	/* major is 0 */
	off = le32_to_cpu(pe->lba_count);

	pagefile_add_blocks(off);
}
#endif

/* Take care as we want this routine to be callable post boot in some confgs in future */
uint_fast8_t tinydisk_setup(uint16_t dev)
{
	uint32_t *lba = td_lba[dev];
	uint_fast8_t n;
	uint_fast8_t c = 0;
	boot_record_t *br = (boot_record_t *) tmpbuf();
	partition_table_entry_t *pe = br->partition;

	/* Platform custom partitions */
#ifdef CONFIG_TD_CUSTOMPART
	n = td_plt_setup(dev, lba, (void *)br);
	if (n < 2)
		return n;
#endif
	udata.u_block = 0;
	udata.u_nblock = 1;
	udata.u_dptr = (void *) br;
	if (td_read(dev << 4, 0, 0) != BLKSIZE) {
		tmpfree(br);
		return 0;
	}
	kprintf("hd%c: ", 'a' + dev);

	if (le16_to_cpu(br->signature) == MBR_SIGNATURE) {
		n = 0;
		while (n < 4) {
			if (pe->type_chs_last[0]) {
				kprintf("hd%c%d ", 'a' + dev, ++c);
				*++lba = le32_to_cpu(pe->lba_first);
			}
#if defined(CONFIG_DYNAMIC_SWAP) || defined(CONFIG_DYNAMIC_PAGE)
			if (pe->type_chs_last[0] == FUZIX_SWAP)
				swap_found((dev << 4) | c, pe);
#endif
			n++;
			pe++;
		}
	}
	tmpfree(br);
	kputchar('\n');
	return 1;
}

uint8_t td_next;
static uint8_t warned;

int td_register(uint_fast8_t unit, td_xfer rwop, td_ioc iop, uint_fast8_t parts)
{
	if (td_next == CONFIG_TD_NUM) {
		if (!warned++)
                    kprintf(": no more device slots.\n");
		return -2;
	}
	td_op[td_next] = rwop;
	td_iop[td_next] = iop;
	td_unit[td_next] = unit;
	if (parts) {
		if (!tinydisk_setup(td_next)) {
			td_op[td_next] = NULL;
			return -1;
		}
	}
	return td_next++;
}
