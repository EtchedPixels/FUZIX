#ifndef __GPT_DOT_H__
#define __GPT_DOT_H__

#include <sys/compiler.h>

/* Copyright 2018 Tormod Volden */

#define MBR_GPT_PROTECTED_TYPE 0xEE

/* FUZIX root partition type */
#define GPT_FUZIX_ROOT_UUID 63ce1ba5-46ab-49bd-abf4-2859975849e1
#define GPT_FUZIX_ROOT_UUID_16BIT 1ba5

typedef struct __packed {
	uint8_t	signature[8];
	uint32_t	revision;
	uint32_t	header_size;
	uint32_t	header_crc;
	uint8_t	reserved[4];
	uint32_t	current_lba_lsb; /* 64 bit LSB */
	uint32_t	current_lba_msb;
	uint32_t	backup_lba_lsb;
	uint32_t	backup_lba_msb;
	uint32_t	first_usable_lba_lsb;
	uint32_t	first_usable_lba_msb;
	uint32_t	last_usable_lba_lsb;
	uint32_t	last_usable_lba_msb;
	uint8_t	disk_guid[16];
	uint32_t	table_starting_lba_lsb;
	uint32_t	table_starting_lba_msb;
	uint32_t	num_part_entries;
	uint32_t	size_part_entry;
	uint32_t	crc_part_array;
	/* reserved to end of block */
} gpt_header_t;

typedef struct __packed {
	uint8_t	type_guid[16];
	uint8_t	uniq_part_guid[16];
	uint32_t	first_lba_lsb;
	uint32_t	first_lba_msb;
	uint32_t	last_lba_lsb;
	uint32_t	last_lba_msb;
	uint8_t	attr_flags[8];
	uint8_t	part_name[72]; /* 36 UTF_16LE */
} gpt_partition_entry_t;

#endif /* __GPT_DOT_H__ */
