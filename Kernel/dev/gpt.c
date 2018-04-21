/*
 * Copyright 2018 Tormod Volden
 *
 */

#include <kernel.h>
#include <printf.h>
#include <blkdev.h>
#include <config.h>
#include "mbr.h"
#include "gpt.h"

#ifdef CONFIG_GPT

void kputs_utf16le(uint8_t *s)
{
	int i;
	for (i = 0; s[i] || s[i+1]; i += 2) {
		if (s[i+1])
			kputchar('!');
		else
			kputchar(s[i] & 0x7f);
	}
}

int parse_gpt_part_entry(gpt_partition_entry_t *entry, uint8_t next)
{
	int i;
	uint8_t zero_check = 0;
	for (i = 0; i < 16; i++)
		zero_check |= entry->type_guid[i];
	if (zero_check == 0)
		return 0;
	if (entry->last_lba_msb != 0) {
		kputs("Partition out of reach\n");
		return 0;
	}

	blk_op.blkdev->lba_first[next] = le32_to_cpu(entry->first_lba_lsb);
	blk_op.blkdev->lba_count[next] = le32_to_cpu(entry->last_lba_lsb) - le32_to_cpu(entry->first_lba_lsb);

	kprintf("%d ", next + 1);

	/* on the verbose side */
	/* kprintf("(%lx-%lx) ", le32_to_cpu(entry->first_lba_lsb), le32_to_cpu(entry->last_lba_lsb)); */

	/* only print first bits of type guid, in conventional order */
	for (i = 3; i >= 0; i--)
		kprintf("%2x", entry->type_guid[i]);
	kputchar(' ');
	kputs_utf16le(entry->part_name);
	kputchar('\n');

	return 1; /* count me */
}

void parse_gpt(uint8_t *buf, uint8_t i_mbr)
{
	int i;
	uint32_t entry_block;
	int num_part;
	gpt_header_t *gpt_header = (gpt_header_t*) buf;
	uint8_t next = 0;
	uint8_t counted;

	kputs("GPT:\n");

	/* read block 1 into buffer, mbr.c has set it all up */
	blk_op.lba = 1;
	blk_op.blkdev->transfer();

	if (strcmp((const char *) gpt_header->signature, "EFI PART")) {
		kputs("No valid GPT header in block 1\n");
		/* TODO look for backup header */
		return;
	}

	if (gpt_header->table_starting_lba_msb != 0) {
		kputs("GPT out of reach\n");
		return;
	}

	entry_block = le32_to_cpu(gpt_header->table_starting_lba_lsb);

	if (le32_to_cpu(gpt_header->size_part_entry) != 128) {
		kputs("Unsupported GPT entry size\n");
		return;
	}

	num_part = (int) le32_to_cpu(gpt_header->num_part_entries);

	for (i = 0; i < num_part && next < MAX_PARTITIONS; i++) {
		gpt_partition_entry_t *entry;
		/* each block has 4 partition entries */
		if (i % 4 == 0) {
			blk_op.lba = entry_block++;
			blk_op.blkdev->transfer();
		}
		entry = (gpt_partition_entry_t *) buf + (i % 4);
		counted = parse_gpt_part_entry(entry, next);
		next += counted;
	}
}

#endif /* CONFIG_GPT */
