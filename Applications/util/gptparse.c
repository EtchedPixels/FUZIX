/*
 * Copyright 2018 Tormod Volden
 *
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "mbr.h"
#include "gpt.h"

#define MAX_PARTITIONS 255 /* for testing */

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
uint32_t le32_to_cpu(uint32_t v)
{
  uint8_t *p = (uint8_t *)&v;
  return ((uint32_t)p[3] << 24) | ((uint32_t)p[2] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[0];
}
uint16_t le16_to_cpu(uint16_t v)
{
  uint8_t *p = (uint8_t *)&v;
  return ((uint16_t)p[1] << 8) | (uint16_t)p[0];
}
#else
#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)
#endif

void puts_utf16le(uint8_t *s)
{
	int i;
	for (i = 0; s[i] || s[i+1]; i += 2) {
		if (s[i+1])
			putchar('!');
		else
			putchar(s[i] & 0x7f);
	}
}

int is_pmbr(uint8_t *buf)
{
	int i;
	boot_record_t *mbr = (boot_record_t*) buf;

	/* check for MBR magic */
	if (le16_to_cpu(mbr->signature) != MBR_SIGNATURE) {
		puts("No MBR signature found\n");
		return 0;
	}
	/* check MBR partition table for 0xEE type */
	for (i = 0; i < MBR_ENTRY_COUNT; i++)
		if (mbr->partition[i].type_chs_last[0] == MBR_GPT_PROTECTED_TYPE)
			return 1;
	/* TODO check partition start == 1 (whole disk) */
	puts("No GPT protected partition type in MBR table\n");
	return 0;
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
		puts("Partition out of reach\n");
		return 0;
	}

	printf("%2d (%08lx-%08lx) ", next + 1, le32_to_cpu(entry->first_lba_lsb), le32_to_cpu(entry->last_lba_lsb));
	/* only print first bits of type guid, in conventional order */
	for (i = 3; i >= 0; i--)
		printf("%02x", entry->type_guid[i]);
	putchar(' ');
	puts_utf16le(entry->part_name);
	putchar('\n');
	return 1; /* count me */
}

int parse_gpt(uint8_t *buf)
{
	int i;
	uint32_t entry_block;
	int num_part;
	gpt_header_t *gpt_header = (gpt_header_t*) buf;
	uint8_t next = 0;
	int counted;

	/* read block 1 into buffer */
	read(0, buf, 512);

	if (strcmp((const char *) gpt_header->signature, "EFI PART")) {
		puts("No valid GPT header in block 1\n");
		/* TODO look for backup header */
		return 0;
	}

	if (gpt_header->table_starting_lba_msb != 0) {
		puts("GPT out of reach\n");
		return 0;
	}

	entry_block = le32_to_cpu(gpt_header->table_starting_lba_lsb);

	if (le32_to_cpu(gpt_header->size_part_entry) != 128) {
		puts("GPT entry size not supported\n");
		return 0;
	}

	num_part = (int) le32_to_cpu(gpt_header->num_part_entries);

	printf("GPT has max %d entries\n", num_part);

	for (i = 0; i < num_part && next < MAX_PARTITIONS; i++) {
		gpt_partition_entry_t *entry;
		/* each block has 4 partition entries */
		if (i % 4 == 0) {
			/* read entry_block++ into buf */
			read(0, buf, 512);
		}
		entry = (gpt_partition_entry_t *) buf + (i % 4);
		counted = parse_gpt_part_entry(entry, next);
		if (!counted)
			break;
		next += counted;
	}
	return next;
}

int main(void)
{
	uint8_t buf[512];

	/* read block 0 into buffer */
	read(0, buf, 512);

	if (is_pmbr(buf) && parse_gpt(buf))
		return 0;
	puts("No GPT Protected MBR found\n");
	return 1;
}
