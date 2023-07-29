#ifndef TINYIDE_H
#define TINYIDE_H

#ifndef TD_IDE_NUM
#define TD_IDE_NUM	1		/* One port, max two devices (master/slave) per port */
#endif

/* SDCC does I/O space weirdly. An __sfr __at x is a reference to the space
   not a pointer */
#if defined(CONFIG_TINYIDE_SDCCPIO)
#define ide_read(x)	(x)
#define ide_write(x,y)	(x) = (y)
#elif defined(CONFIG_TINYIDE_INDIRECT)
extern uint8_t ide_read(uint_fast8_t r);
extern void ide_write(uint_fast8_t r, uint_fast8_t v); 
#else
#define	ide_read(x)	(*(x))
#define ide_write(x,y)	(*(x) = (y))
#endif

/* Assembler glue */
extern void devide_read_data(uint8_t *p);
extern void devide_write_data(uint8_t *p);
int ide_xfer(uint_fast8_t unit, bool is_read, uint32_t lba, uint8_t * dptr);
extern uint8_t ide_dev[TD_IDE_NUM];
extern uint8_t ide_unit;

void ide_probe(void);

#endif
