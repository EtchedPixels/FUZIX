#ifndef TINYIDE_H
#define TINYIDE_H

#ifndef TD_IDE_NUM
#define TD_IDE_NUM	2		/* One port, max two devices (master/slave) per port */
#endif

#ifndef IDE_IS_8BIT
#define IDE_IS_8BIT(x)	0
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

extern uint8_t ide_spt[TD_IDE_NUM];
extern uint8_t ide_heads[TD_IDE_NUM];
extern uint16_t ide_cyls[TD_IDE_NUM];

/* Assembler glue */
extern void devide_read_data(uint8_t *p);
extern void devide_write_data(uint8_t *p);
int ide_xfer(uint_fast8_t unit, bool is_read, uint32_t lba, uint8_t * dptr);
int ide_chs_xfer(uint_fast8_t unit, bool is_read, uint32_t lba, uint8_t * dptr);
int ide_ioctl(uint_fast8_t dev, uarg_t request, char *unused);
extern uint8_t ide_dev[TD_IDE_NUM];
extern uint8_t ide_unit;

void ide_probe(void);
void ide_std_reset(void);

void ide_reset(void);	/* Optional caller provided */

#endif
