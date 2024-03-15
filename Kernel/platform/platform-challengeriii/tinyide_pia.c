/*
 *	6820 PIA as CF adapter
 *
 *	Port A data (100ohm resistors)
 *	Port B control
 *	7:	(reset)
 *	6:	WR
 *	5:	RD
 *	4:	CS1
 *	3:	CS0
 *	2:}
 *	1:}	register
 *	0:}
 */

#include <kernel.h>
#include <tinydisk.h>
#include <tinyide.h>
#include <plt_ide.h>

/* Using CD00 for the moment: change in osi.s as well */
volatile uint8_t *pia = (volatile uint8_t *) 0xCD00;

void ide_set_w(void)
{
	pia[1] = 0x34;		/* Write direction reigster */
	pia[0] = 0xFF;
	pia[1] = 0x30;		/* CAS2 pass throguh, no interrupts */
}

void ide_set_r(void)
{
	pia[1] = 0x34;		/* Write direction reigster */
	pia[0] = 0x00;
	pia[1] = 0x30;		/* CAS2 pass throguh, no interrupts */
}

uint8_t ide_read(uint_fast8_t r)
{
	uint_fast8_t v;
	/* Active low control lines */
	pia[2] = PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET | (r & 7);
	pia[2] = PIAIDE_CS1 | PIAIDE_W | PIAIDE_RESET | (r & 7);
	v = pia[0];
	pia[2] = PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET | (r & 7);
	pia[2] = PIAIDE_CS0 | PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET;
	return v;
}

void ide_write(uint_fast8_t r, uint_fast8_t v)
{
	ide_set_w();
	/* Active low control lines */
	pia[2] = PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET | (r & 7);
	pia[0] = v;
	pia[2] = PIAIDE_CS1 | PIAIDE_R | PIAIDE_RESET | (r & 7);
	pia[2] = PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET | (r & 7);
	pia[2] = PIAIDE_CS0 | PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET;
	ide_set_r();
}

void ide_setup(void)
{
	/* Set up control port */
	ide_set_r();
	pia[3] = 0x34;
	pia[2] = 0xFF;		/* Write */
	pia[3] = 0x30;
	pia[2] = PIAIDE_CS0 | PIAIDE_CS1 | PIAIDE_R | PIAIDE_W | PIAIDE_RESET;
}
