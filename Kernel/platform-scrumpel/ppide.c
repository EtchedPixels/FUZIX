/*
 *	Based on the retrobrew PPIDE but Scrumpel does it's own thing
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

#ifdef CONFIG_PPIDE

__sfr __at (PPIDE_BASE + 0x00) ppi_port_a;   /* IDE bus LSB */
__sfr __at (PPIDE_BASE + 0x01) ppi_port_b;   /* IDE bus control signals */
__sfr __at (PPIDE_BASE + 0x03) ppi_control;  /* 8255 command register */

void ppide_init(void)
{
    ppi_control = PPIDE_PPI_BUS_READ;
    ppi_port_b = ide_reg_status;
}

uint_fast8_t devide_readb(uint_fast8_t regaddr)
{
    uint8_t r;

    /* note: ppi_control should contain PPIDE_PPI_BUS_READ already */
    ppi_port_b = regaddr | PPIDE_CS0_LINE;
    ppi_port_b = regaddr;
    ppi_port_b = regaddr | PPIDE_RD_LINE; /* begin /RD pulse */
    ppi_port_b = regaddr;	 /* end /RD pulse */
    r = ppi_port_a;
    ppi_port_b = regaddr & PPIDE_CS0_LINE;
    return r;
}

void devide_writeb(uint_fast8_t regaddr, uint_fast8_t value)
{
    ppi_control = PPIDE_PPI_BUS_WRITE;
    ppi_port_b = regaddr | PPIDE_CS0_LINE;
    ppi_port_b = regaddr;
    ppi_port_a = value;
    ppi_port_b = regaddr | PPIDE_WR_LINE;
    ppi_port_b = regaddr;
    ppi_port_b = regaddr & PPIDE_CS0_LINE;
    ppi_control = PPIDE_PPI_BUS_READ;
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/
COMMON_MEMORY

/* Will need updating if we add swap over CF card */

#ifdef SWAP_DEV
#error "please fix me"
#endif

void devide_read_data(void) __naked
{
    __asm
            ld a, #ide_reg_data
            ld bc, #PPIDE_BASE+1                    ; select control lines and count
            out (c), a                              ; select IDE data register
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld d, #ide_reg_data                     ; register address
            ld e, #ide_reg_data | PPIDE_RD_LINE     ; register address with /RD asserted
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            or a                                    ; test is_user
            push af                                 ; save flags
            call nz, map_proc_always             ; map user memory first if required
goread:     ; now we do the transfer
            out (c), e                              ; assert /RD
    	    out (c), d                              ; de-assert /RD
    	    dec c
            ini                                     ; read byte from LSB
            jr z, goread_next
            inc c                                   ; control lines
            jr goread                               ; (delay) next byte
goread_next:
            inc c
            out (c), d
goread2:    ; next 256 bytes
            out (c), e                              ; assert /RD
            out (c), d				    ; de-assert /RD
	    dec c
            ini                                     ; read byte from LSB
            jr z, goread_done
            inc c                                   ; control lines
            jr goread2                              ; (delay) next byte
            ; read completed
goread_done:
            inc	c
	    res 7,d
            out (c), d				    ; de-assert /CS to make the LED go out
            pop af                                  ; recover is_user test result
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void devide_write_data(void) __naked
{
    __asm
            ld bc, #PPIDE_BASE+1                    ; select control lines
            ld a, #ide_reg_data
            out (c), a                              ; select data register
            ld a, #PPIDE_PPI_BUS_WRITE
            out (PPIDE_BASE+3), a                   ; 8255A port A to output mode
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld d, #ide_reg_data                     ; register address
            ld e, #ide_reg_data | PPIDE_WR_LINE     ; register address with /WR asserted
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            or a                                    ; test is_user
            push af                                 ; save flags
            call nz, map_proc_always             ; map user memory first if required
gowrite:    ; now we do the transfer
            dec c				    ; data (port A)
            outi                                    ; write byte to LSB
            jr z, gowrite_next
            inc c                                   ; up to MSB
            out (c), e                              ; assert /WR
	    out (c), d			            ; de-assert /WR
            jp gowrite                              ; (delay) next word
gowrite_next:
            inc c
            out (c), e
gowrite2:   ; next 256 bytes
            out (c), d                              ; de-assert /WR
            dec c
            outi                                    ; write byte to LSB
            jr z, gowrite_done
            inc c                                   ; up to control lines
            out (c), e                              ; assert /WR
            jp gowrite2                             ; (delay) next word
gowrite_done:
            inc c
            out (c), e
            ; write completed
            out (c), d                              ; de-assert /WR
            res 7,d
            out (c), d				    ; de-assert /CS to make the LED go out
            ld a, #PPIDE_PPI_BUS_READ
            out (PPIDE_BASE+3), a                   ; 8255A ports A, B to read mode
            pop af                                  ; recover is_user test result
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

#endif
