/* 2015-04-24 WRS: devide glue functions for PPIDE
   This may need to change for some more unusual platforms but for Z80 based
   Retrobrew / N8VEM systems should work as is */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>

#ifdef CONFIG_TINYIDE_PPI

__sfr __at (PPIDE_BASE + 0x00) ppi_port_a;   /* IDE bus LSB */
__sfr __at (PPIDE_BASE + 0x01) ppi_port_b;   /* IDE bus MSB */
__sfr __at (PPIDE_BASE + 0x02) ppi_port_c;   /* IDE bus control signals */
__sfr __at (PPIDE_BASE + 0x03) ppi_control;  /* 8255 command register */

void ppide_init(void)
{
    ppi_control = PPIDE_PPI_BUS_READ;
    ppi_port_c = status;
}

uint_fast8_t ide_read(uint_fast8_t regaddr)
{
    uint8_t r;

    /* note: ppi_control should contain PPIDE_PPI_BUS_READ already */
    ppi_port_c = regaddr;
    ppi_port_c = regaddr | PPIDE_RD_LINE; /* begin /RD pulse */
    r = ppi_port_a;
    ppi_port_c = regaddr;	 /* end /RD pulse */
    return r;
}

void ide_write(uint_fast8_t regaddr, uint_fast8_t value)
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

void devide_read_data(uint8_t *addr) __naked
{
    __asm
#ifdef CONFIG_BANKED
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
#else
            pop de
            pop hl
            push hl
            push de
#endif
            ld a, #data
            ld c, #PPIDE_BASE+2                     ; select control lines
            out (c), a                              ; select IDE data register
            ld d, #data				    ; register address
            ld e, #data | PPIDE_RD_LINE		    ; register address with /RD asserted
            ld b, #0                                ; setup count
            ld a, (_td_raw)
#ifdef SWAPDEV            
            cp #2				    ; swap ?
            jr nz, not_swapin			    ; nope
            ld a, (_td_page)			    ; get the page to use
            call map_for_swap			    ; map it if needed
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, rd_kernel			    ; kernel buffer read ?
            call map_proc_always		    ; map the process memory
            jr doread
rd_kernel:
            call map_buffers			    ; ensure the buffers are mapped
doread:
            ld a, #PPIDE_BASE+0                     ; I will be needing this later
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
            jp map_kernel                           ; map kernel then return
    __endasm;
}

void devide_write_data(uint8_t *addr) __naked
{
    __asm
#ifdef CONFIG_BANKED
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
#else
            pop de
            pop hl
            push hl
            push de
#endif
            ld c, #PPIDE_BASE+2                     ; select control lines
            ld a, #data
            out (c), a                              ; select data register
            ld a, #PPIDE_PPI_BUS_WRITE
            inc c                                   ; up to 8255A command register
            out (c), a                              ; 8255A ports A, B to output mode
            dec c                                   ; back down to the control lines
            ld d, #data                 	    ; register address
            ld e, #data | PPIDE_WR_LINE		    ; register address with /WR asserted
            ld b, #0                                ; setup count
            ld a, (_td_raw)			    ; blkparam.is_user

#ifdef SWAPDEV
	    cp #2				    ; swap out ?
	    jr nz, not_swapout			    ; skip if not
	    ld a,(_td_page)			    ; get page to swap
	    call map_for_swap			    ; map it so we can write it out
	    jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
	    jr z, wr_kernel			    ; writing from kernel ?
	    call map_proc_always	 	    ; map user space
	    jr dowrite
wr_kernel:
	    call map_buffers			    ; map the disk buffers
dowrite:
            ld a, #PPIDE_BASE+0                     ; I will be needing this later
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
            jp map_kernel                           ; map kernel then return
    __endasm;
}

#endif
