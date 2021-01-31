#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../lib/dhara/map.h"
#include "../lib/dhara/nand.h"

#define FLASH_SIZE (2*1024*1024)

static uint8_t flashdata[FLASH_SIZE];
static struct dhara_map dhara;
static const struct dhara_nand nand = 
{
	.log2_page_size = 9,
	.log2_ppb = 12 - 9,
	.num_blocks = FLASH_SIZE / 4096,
};

static uint8_t journal_buf[512];

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
                     dhara_error_t *err)
{
	memset(flashdata + (b*4096), 0xff, 512);
	*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data,
                    dhara_error_t *err)
{
	memcpy(flashdata + (p*512), data, 512);
	*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
					size_t offset, size_t length,
                    uint8_t *data,
                    dhara_error_t *err)
{
	if (p >= (FLASH_SIZE/512))
	{
		*err = DHARA_E_BAD_BLOCK;
		return -1;
	}

	memcpy(data, flashdata + (p*512) + offset, length);
	*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_is_bad(const struct dhara_nand* n, dhara_block_t b)
{
	return 0;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b) {}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
	const uint8_t* ptr = flashdata + (p*512);
	for (int i=0; i<512; i++)
		if (ptr[i] != 0xff)
			return 0;

	return 1;
}

int dhara_nand_copy(const struct dhara_nand *n,
                    dhara_page_t src, dhara_page_t dst,
                    dhara_error_t *err)
{
	const uint8_t* psrc = flashdata + (src*512);
	uint8_t* pdst = flashdata + (dst*512);
	memcpy(pdst, psrc, 512);
	*err = DHARA_E_NONE;
	return 0;
}

int main(int argc, const char* argv[])
{

	dhara_map_init(&dhara, &nand, journal_buf, 128);
	dhara_error_t err = DHARA_E_NONE;
	dhara_map_resume(&dhara, &err);
	printf("Number of physical erase blocks: %d\n", nand.num_blocks);

	uint32_t lba = dhara_map_capacity(&dhara);
	printf("Maximum logical size: %dkB (%d sectors)\n", lba / 2, lba);

	FILE* inf = fopen(argv[1], "rb");
	fseek(inf, 0, SEEK_END);
	int sectors = ftell(inf) / 512;
	fseek(inf, 0, SEEK_SET);
	if (sectors > lba)
	{
		fprintf(stderr, "Logical image too big (%d > %d)\n",
			sectors, lba);
		exit(1);
	}

	for (int sectorno=0; sectorno<sectors; sectorno++)
	{
		uint8_t buffer[512] = {};
		fread(buffer, 512, 1, inf);
		err = DHARA_E_NONE;
		dhara_map_write(&dhara, sectorno, buffer, &err); 
		if (err != DHARA_E_NONE)
		{
			fprintf(stderr, "FTL error %d\n", err);
			exit(1);
		}
	}
	fclose(inf);

	err = DHARA_E_NONE;
	dhara_map_gc(&dhara, &err);
	uint32_t used = dhara_map_size(&dhara);
	printf("Used space: %dkB (%d sectors)\n", used / 2, used);

	FILE* outf = fopen(argv[2], "wb");
	fwrite(flashdata, 1, FLASH_SIZE, outf);
	fclose(outf);
	return 0;
}

