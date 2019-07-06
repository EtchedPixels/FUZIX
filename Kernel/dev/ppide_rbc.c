/* 2015-04-24 WRS: devide glue functions for PPIDE
   This may need to change for some more unusual platforms but for Z80 based
   Retrobrew / N8VEM systems should work as is */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#ifdef CONFIG_PPIDE

/* Use the driver names unless we are in a platform with multiple IDE
   configurations at once. In that case we will be called by the wrapper
   it provides */

#ifndef CONFIG_MULTI_IDE
#define ppide_readb		devide_readb
#define ppide_writeb		devide_writeb
#define ppide_read_data		devide_read_data
#define ppide_write_data	devide_write_data
#endif

__sfr __at (PPIDE_BASE + 0x00) ppi_port_a;   /* IDE bus LSB */
__sfr __at (PPIDE_BASE + 0x01) ppi_port_b;   /* IDE bus MSB */
__sfr __at (PPIDE_BASE + 0x02) ppi_port_c;   /* IDE bus control signals */
__sfr __at (PPIDE_BASE + 0x03) ppi_control;  /* 8255 command register */

void ppide_init(void)
{
    ppi_control = PPIDE_PPI_BUS_READ;
    ppi_port_c = ide_reg_status;
}

uint_fast8_t ppide_readb(uint_fast8_t regaddr)
{
    uint8_t r;

    /* note: ppi_control should contain PPIDE_PPI_BUS_READ already */
    ppi_port_c = regaddr;
    ppi_port_c = regaddr | PPIDE_RD_LINE; /* begin /RD pulse */
    r = ppi_port_a;
    ppi_port_c = regaddr;	 /* end /RD pulse */
    return r;
}

void ppide_writeb(uint_fast8_t regaddr, uint_fast8_t value)
{
    ppi_control = PPIDE_PPI_BUS_WRITE;
    ppi_port_c = regaddr;
    ppi_port_a = value;
    ppi_port_b = 0;
    ppi_port_c = regaddr | PPIDE_WR_LINE;
    /* FIXME: check timing */
    ppi_port_c = regaddr;
    ppi_control = PPIDE_PPI_BUS_READ;
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/
COMMON_MEMORY

void ppide_read_data(void) __naked
{
    __asm
            ld a, #ide_reg_data
            ld c, #PPIDE_BASE+2                     ; select control lines
            out (c), a                              ; select IDE data register
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld d, #ide_reg_data                     ; register address
            ld e, #ide_reg_data | PPIDE_RD_LINE     ; register address with /RD asserted
            ld b, #0                                ; setup count
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            or a                                    ; test is_user
            push af                                 ; save flags
            ld a, #PPIDE_BASE+0                     ; I will be needing this later
            call nz, map_process_always             ; map user memory first if required
goread:     ; now we do the transfer
            out (c), e                              ; assert /RD
            ld c, a                                 ; PPIDE_BASE
            ini                                     ; read byte from LSB
            inc c                                   ; up to MSB
            ini                                     ; read byte from MSB
            inc c                                   ; control lines
            out (c), d                              ; de-assert /RD
            inc b                                   ; (delay) counteract second ini instruction
            jr nz, goread                           ; (delay) next word
            ; read completed
            pop af                                  ; recover is_user test result
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void ppide_write_data(void) __naked
{
    __asm
            ld c, #PPIDE_BASE+2                     ; select control lines
            ld a, #ide_reg_data
            out (c), a                              ; select data register
            ld a, #PPIDE_PPI_BUS_WRITE
            inc c                                   ; up to 8255A command register
            out (c), a                              ; 8255A ports A, B to output mode
            dec c                                   ; back down to the control lines
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld d, #ide_reg_data                     ; register address
            ld e, #ide_reg_data | PPIDE_WR_LINE     ; register address with /WR asserted
            ld b, #0                                ; setup count
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            or a                                    ; test is_user
            push af                                 ; save flags
            ld a, #PPIDE_BASE+0                     ; I will be needing this later
            call nz, map_process_always             ; map user memory first if required
gowrite:    ; now we do the transfer
            out (c), d                              ; de-assert /WR
            ld c, a                                 ; PPIDE_BASE
            outi                                    ; write byte to LSB
            inc c                                   ; up to MSB
            outi                                    ; write byte to MSB
            inc c                                   ; up to control lines
            out (c), e                              ; assert /WR
            inc b                                   ; (delay) offset to counteract second outi instruction
            jr nz, gowrite                          ; (delay) next word
            ; write completed
            out (c), d                              ; de-assert /WR
            ld a, #PPIDE_PPI_BUS_READ
            inc c                                   ; up to 8255A command register
            out (c), a                              ; 8255A ports A, B to read mode
            pop af                                  ; recover is_user test result
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

#endif
