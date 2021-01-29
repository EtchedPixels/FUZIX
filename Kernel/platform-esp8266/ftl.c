#include <stdint.h>
#include <stddef.h>
#include "printf.h"
#include "ftl.h"

#define FLASH_BLOCK_SIZE 4096
#define FLASH_SIZE (44*1024)
#define FLASH_BLOCKS (FLASH_SIZE/FLASH_BLOCK_SIZE)

union block
{
	struct
	{
		uint32_t erasecount;
		uint16_t logical;
	} metadata;
	uint8_t data[512];
};

static union block buffer;
static uint16_t logicalToPhysical[FLASH_BLOCKS-1];
static uint32_t sparePhysicalBlock;
static uint32_t roundRobinBlock;

/* Initialise the FTL layer. */
int ftl_init(void)
{
	for (unsigned logical=0; logical<FLASH_BLOCKS-1; logical++)
		logicalToPhysical[logical] = 0xfffe;

	for (unsigned physical=0; physical<FLASH_BLOCKS; physical++)
	{
		raw_flash_read(physical, -1, buffer.data);
		if (buffer.metadata.logical == 0xffff)
			sparePhysicalBlock = physical;
		else
			logicalToPhysical[buffer.metadata.logical] = physical;
	}

	for (unsigned logical=0; logical<FLASH_BLOCKS-1; logical++)
	{
		if (logicalToPhysical[logical] == 0xfffe)
		{
			kprintf("bad flash FTL data\n");
			return -1;
		}
	}

	return (FLASH_BLOCKS-1) * 7;
}

/* Read a 512 byte logical sector. */
int ftl_read(uint32_t logical, int sector, uint8_t* buffer)
{
	unsigned physical = logicalToPhysical[logical];
	kprintf("read %x.%d / %x\n", logical, sector, physical);
	return raw_flash_read(physical, sector, buffer);
}

static void copy_physical_sector(uint32_t src, uint32_t dest, int sector)
{
	raw_flash_read(src, sector, buffer.data);
	raw_flash_write(dest, sector, buffer.data);
}

/* Write a 512 byte logical sector. */
int ftl_write(uint32_t logical, int sector, const uint8_t* buffer)
{
	unsigned physical = logicalToPhysical[logical];
	kprintf("write %x.%d / %x\n", logical, sector, physical);

	if ((roundRobinBlock != sparePhysicalBlock) && (roundRobinBlock != physical))
	{
		/* Evict whatever's at the round robin block. */

		for (int i=-1; i<7; i++)
			copy_physical_sector(roundRobinBlock, sparePhysicalBlock, i);
		raw_flash_erase(roundRobinBlock);
		sparePhysicalBlock = roundRobinBlock;
	}

	for (int i=-1; i<7; i++)
	{
		if (sector == i)
			raw_flash_write(sparePhysicalBlock, sector, buffer);
		else
			copy_physical_sector(physical, sparePhysicalBlock, i);
	}
	raw_flash_erase(physical);
	sparePhysicalBlock = physical;
	
	roundRobinBlock = (roundRobinBlock+1) % FLASH_BLOCKS;
	return 0;
}

