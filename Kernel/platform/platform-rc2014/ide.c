#include <kernel.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>
#include <printf.h>

uint16_t ide_port = 0x0010;		/* 256 count, port 0x10 */
/* Port I/O: Currently Z80 only */

static uint_fast8_t drive;

static void cf_read_data(uint8_t *dptr);
static void cf_write_data(uint8_t *dptr);

uint8_t ide_read(uint_fast8_t reg)
{
    if (ide_unit > 1)	/* PPIDE */
        return ppide_read(reg);
    else if (reg & 0x08)
        return in(ide_port + (reg & 7));
    return 0x00;
}

void ide_write(uint_fast8_t reg, uint_fast8_t val)
{
    if (ide_unit > 1)	/* PPIDE */
        ppide_write(reg, val);
    else if (reg & 0x08)
        out(ide_port + (reg & 7), val);
}

void devide_read_data(uint8_t *dptr)
{
    if (ide_unit > 1)	/* PPIDE */
        ppide_read_data(dptr);
    else
        cf_read_data(dptr);
}

void devide_write_data(uint8_t *dptr)
{
    if (ide_unit > 1)	/* PPIDE */
        ppide_write_data(dptr);
    else
        cf_write_data(dptr);
}

COMMON_MEMORY

static void cf_read_data(uint8_t *dptr) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, (_ide_port)                      ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)			    ; swap
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, rd_kernel
            call map_proc_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
            call sector_dma_in
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

static void cf_write_data(uint8_t *dptr) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, (_ide_port)                      ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)			    ; blkparam.swap_page
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            jr z, wr_kernel
            call map_proc_always		    ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
            call sector_dma_out
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}
