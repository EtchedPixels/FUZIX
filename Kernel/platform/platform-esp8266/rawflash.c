#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include "lib/dhara/nand.h"
#include "rom.h"
#include "printf.h"

#undef DEBUG

#define FLASH_OFFSET 32 /* in 4kB blocks; 128kB */

#define FLASH_ADDRESS(page) \
	((FLASH_OFFSET * 4096) + (page * 512))

/*
 *	These routines live in the iram because the flash executable memory ceases to be
 *	available while we are performing low level flash operations
 */
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
                     dhara_error_t *err)
{
    #if defined DEBUG
        kprintf("erase block %d\n", b);
    #endif
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiEraseSector(b + FLASH_OFFSET);
	Wait_SPI_Idle(sdk_flashchip);
	Cache_Read_Enable(0, 0, 0);
	irqrestore(f);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data,
                    dhara_error_t *err)
{
    #if defined DEBUG
        kprintf("write page %d\n", p);
    #endif
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiWrite(FLASH_ADDRESS(p), data, 512);
	Wait_SPI_Idle(sdk_flashchip);
	Cache_Read_Enable(0, 0, 0);
	irqrestore(f);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
					size_t offset, size_t length,
                    uint8_t *data,
                    dhara_error_t *err)
{
    #if defined DEBUG
        kprintf("read page %d\n", p);
    #endif
	irqflags_t f = di();
	Cache_Read_Disable();
	SpiUnlock();
	SpiRead(FLASH_ADDRESS(p) + offset, data, length);
	Wait_SPI_Idle(sdk_flashchip);
	Cache_Read_Enable(0, 0, 0);
	irqrestore(f);
	if (err)
		*err = DHARA_E_NONE;
	return 0;
}

/* vim: sw=4 ts=4 et: */


