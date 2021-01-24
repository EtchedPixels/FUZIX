#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include "ftl.h"
#include "rom.h"

#define FLASH_OFFSET 256

#define FLASH_ADDRESS(p, s) \
	(((p + FLASH_OFFSET)*4096) + ((s+1) * 512))

int raw_flash_erase(uint32_t physical)
{
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiEraseSector(physical + 256);
	Cache_Read_Enable(0, 0, 1);
	irqrestore(f);
	return 0;
}

int raw_flash_write(uint32_t physical, int sector, const uint8_t* buffer)
{
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiWrite(FLASH_ADDRESS(physical, sector), buffer,
		(sector == -1) ? 6 : 512);
	Cache_Read_Enable(0, 0, 1);
	irqrestore(f);
	return 0;
}

int raw_flash_read(uint32_t physical, int sector, uint8_t* buffer)
{
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiRead(FLASH_ADDRESS(physical, sector), buffer,
		(sector == -1) ? 6 : 512);
	Cache_Read_Enable(0, 0, 1);
	irqrestore(f);
	return 0;
}

