/*
 *	Atari hard disk boot sectors
 *
 *	This is an initial simple minded parser. The original format was extended
 *	in two ways at least. ICD stuffed extra partition entries into the end of
 *	the bootcode space, whilst the XGM format changes the partition table to
 *	a linked list of entries where each entry is a partition definition and
 *	pointer to a next block.
 *
 *	For more information see Atari AHDI 3.00 Release Notes
 *
 *	FIXME: currently only works on big endian
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include "ahdi.h"

/*
 *	Check a partition table looks sane. That is the entries have
 *	ASCII and valid sizes. If we find no entries then we don't
 *	consider it valid, because it doesn't matter whether it is or not.
 */
static uint_fast8_t check_sane(struct atari_bootblock *br,
			       struct atari_partition *ap, uint_fast8_t n)
{
	uint_fast8_t i;
	uint_fast8_t found = 0;

	for (i = 0; i < n; i++) {
		if (ap->flags & 1) {
			found++;
			if (!ap->start || !ap->length)
				return 0;
			if (ap->start >= br->size
			    || ap->start + ap->length >= br->size)
				return 0;
			if ((ap->id[0] | ap->id[1] | ap->id[2]) & 0x80)
				return 0;
			if (ap->id[0] < 32 || ap->id[1] < 32
			    || ap->id[2] < 32)
				return 0;
		}
		ap++;
	}
	return found;
}

/*
 *	AHDI 3.0 added extended partitions but as with all of this stuff
 *	had to be different to the PC. GEM extended partitions form a chain.
 *	We follow the chain each of which has a partition and then an XGM
 *	entry pointing the next one
 */
static uint_fast8_t ahdi_extended_partition(struct atari_partition *bap,
					    uint_fast8_t next,
					    uint8_t letter)
{
	struct atari_bootblock *ext = (struct atari_bootblock *) tmpbuf();
	struct atari_partition *ap;
	uint_fast8_t i;

	blk_op.addr = (uint8_t *) ext;
	blk_op.lba = bap->start;

	do {
		if (!blk_op.blkdev->transfer())
			return next;

		ap = ext->part;

		for (i = 0; i < 3; i++) {
			if (ap->flags & 1)
				break;
			ap++;
		}
		if (i == 3)
			break;
		/* Can't just be an XGM */
		if (memcmp(ap->id, "XGM", 3) == 0)
			break;

		/* Ok looks believable */
		/* TODO : check we don't overrun table */
		blk_op.blkdev->lba_first[next] = bap->start + ap->start;
		blk_op.blkdev->lba_count[next] = ap->length;
		next++;
		kprintf("hd%c%d (%c%c%c) ",
			letter, next, ap->id[0], ap->id[1], ap->id[2]);
		blk_op.lba = ap[1].start;
	}
	while((ap[1].flags & 1) && memcmp(ap[1].id, "XGM", 3) == 0);
	tmpfree(ext);
	return next;
}

static uint_fast8_t ahdi_install(struct atari_partition *ap,
				 uint_fast8_t n, uint_fast8_t next,
				 uint8_t letter)
{
	uint_fast8_t i;

	for (i = 0; i < n; i++) {
		if (ap->flags & 1) {
			/* TODO : check we don't overrun table */
			blk_op.blkdev->lba_first[next] = ap->start;
			blk_op.blkdev->lba_count[next] = ap->length;
			next++;
			kprintf("hd%c%d (%c%c%c) ",
				letter, next, ap->id[0], ap->id[1],
				ap->id[2]);
			/* AHDI 3 adds chains of partitions */
			if (memcmp(ap->id, "XGM", 3) == 0)
				next =
				    ahdi_extended_partition(ap, next,
							    letter);
		}
		ap++;
	}
	return next;
}

void ahdi_parse(uint_fast8_t letter)
{
	struct atari_bootblock *br;
	uint_fast8_t next = 0;

	kprintf("hd%c:", letter);

	br = (struct atari_bootblock *) tmpbuf();

	blk_op.is_read = true;
	blk_op.is_user = false;
	blk_op.addr = (uint8_t *) br;
	blk_op.lba = 0;

	blk_op.nblock = 1;
	if (!blk_op.blkdev->transfer())
		goto none;

	if (!check_sane(br, br->part, 8))
		goto none;
	/* Probably a valid partition table */
	next = ahdi_install(br->part, 4, next, letter);
	/* Look for an ICD partition table */
	if (check_sane(br, br->icd, 8))
		next = ahdi_install(br->icd, 8, next, letter);
      none:
	tmpfree(br);
	kputchar('\n');
}
