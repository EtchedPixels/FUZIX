#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include "mini_ide.h"

#define data	((volatile uint8_t *)0xFF50)
#define error	((volatile uint8_t *)0xFF51)
#define count	((volatile uint8_t *)0xFF52)
#define sec	((volatile uint8_t *)0xFF53)
#define cyll	((volatile uint8_t *)0xFF54)
#define cylh	((volatile uint8_t *)0xFF55)
#define devh	((volatile uint8_t *)0xFF56)
#define status	((volatile uint8_t *)0xFF57)
#define cmd	((volatile uint8_t *)0xFF57)
#define datal	((volatile uint8_t *)0xFF58)

static timer_t giveup;

static int ide_wait_op(uint8_t mask, uint8_t val)
{
	while((*status & mask) != val) {
		if (timer_expired(giveup)) {
			kputs(" - no response\n");
			return -1;
		}
	}
	return 0;
}

static int ide_wait_nbusy(void)
{
	return ide_wait_op(0x80, 0x00);
}

static int ide_wait_drdy(void)
{
	return ide_wait_op(0x40, 0x40);
}

static int ide_wait_drq(void)
{
	return ide_wait_op(0x08, 0x08);
}


static void ide_identify(int dev, uint8_t *buf)
{
	uint8_t *dptr = buf;
        int i;

	giveup = set_timer_ms(2000);

	kprintf("%x : ", dev);

	if (ide_wait_nbusy() == -1)
		return;
	*devh = dev << 4;	/* Select */
	if (ide_wait_nbusy() == -1)
		return;
	if (ide_wait_drdy() == -1)
		return;
	*cmd = 0xEC;	/* Identify */
	if (ide_wait_drq() == -1)
		return;
        for (i = 0; i < 256; i++) {
		*dptr++ = *data;
		*dptr++ = *datal;
	}
	if (ide_wait_nbusy() == -1)
		return;
	if (*status & 1)
		return;
	if (ide_wait_drdy() == -1)
		return;
	/* Check the LBA bit is set, and print the name */
	dptr = buf + 54;		/* Name info */
	if (*dptr)
		for (i = 0; i < 20; i++) {
			kputchar(dptr[1]);
			kputchar(*dptr);
			dptr += 2;
		}
	if (!(buf[99] & 0x02)) {	/* No LBA ? */
		kputs(" - non-LBA\n");
		return;
	}
	kputs(" - OK\n");
	ide_present |= (1 << dev);
}

void ide_probe(void)
{
	uint8_t *buf = (uint8_t *)tmpbuf();
	/* Issue an EDD if we can - timeout -> no drives */
	/* Now issue an identify for each drive */
	ide_identify(0, buf);
	if (ide_present)
		ide_identify(1, buf);
	tmpfree(buf);
}
