#include <kernel.h>
#include <tinyide.h>

#define pit	((volatile uint8_t *)0xFF000)

#define PGCR	0x00
#define PACR	0x0C
#define PBCR	0x0E
#define PADR	0x10
#define PBDR	0x12
#define PADDR	0x04
#define PBDDR	0x06

uint8_t ide_read(uint8_t addr)
{
    uint8_t r;

    addr ^= 0x18;		/* CS lines are active low */
    pit[PBDR] = 0xE0 | addr;	/* CS low , address */
    pit[PBDR] = 0xA0 | addr;	/* Take \RD low */
    r = pit[PADR];
    pit[PBDR] = 0xF8;		/* All back high */
    return r;
}

void ide_write(uint8_t addr, uint8_t val)
{
    addr ^= 0x18;
    pit[PADR] = val;		/* Value */
    pit[PBDR] = 0xE0 | addr;	/* CS low, address */
    pit[PADDR] = 0xFF;		/* Switch to output, value is now on the bus */
    pit[PBDR] = 0xC0 | addr;	/* Pull \WR low */
    pit[PBDR] = 0xE0 | addr;	/* Pull \WR back high */
    pit[PBDR] = 0xF8;			/* CS back high */
    pit[PADDR] = 0x00;		/* Switch back to input mode */
}

/* TODO : rewrite this pair in hand tuned asm */
void devide_read_data(uint8_t *buf)
{
    unsigned n = 0;
    pit[PBDR] = 0xF0;			/* Data port, CS low */
    while(n++ < 512) {
        pit[PBDR] = 0xB0;		/* Read low */
        *buf++ = pit[PADR];
        pit[PBDR] = 0xF0;		/* and high */
    }
    pit[PBDR] = 0xF8;			/* CS back high */
}

/* TODO : rewrite this pair in hand tuned asm */
void devide_write_data(uint8_t *buf)
{
    unsigned n = 0;
    pit[PBDR] = 0xF0;			/* Data port, CS low */
    pit[PADDR] = 0xFF;
    while(n++ < 512) {
        pit[PADR] = *buf++;		/* Data on bus */
        pit[PBDR] = 0xD0;		/* Write low */
        pit[PBDR] = 0xF0;		/* and high */
    }
    pit[PBDR] = 0xF8;			/* CS back high */
    pit[PADDR] = 0x00;
}

void ide_init(void)
{
    pit[PGCR] = 0x00;
    pit[PADDR] = 0x00;
    pit[PBDDR] = 0xFF;
    pit[PACR] =  0x40;
    pit[PBCR] = 0x40;
}

    