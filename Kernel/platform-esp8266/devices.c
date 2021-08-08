#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <blkdev.h>
#include <tty.h>
#include <devtty.h>
#include <dev/devsd.h>
#include <printf.h>
#include "globals.h"
#include "rom.h"

struct devsw dev_tab[] =  /* The device driver switch table */
{
// minor    open         close        read      write           ioctl
// ---------------------------------------------------------------------
  /* 0: /dev/hd - block device interface */
  {  blkdev_open,   no_close,   blkdev_read,    blkdev_write,	blkdev_ioctl},
  /* 1: /dev/fd - Floppy disk block devices */
  {  no_open,	    no_close,	no_rdwr,	no_rdwr,	no_ioctl},
  /* 2: /dev/tty	TTY devices */
  {  tty_open,     tty_close,   tty_read,  tty_write,  tty_ioctl },
  /* 3: /dev/lpr	Printer devices */
  {  no_open,     no_close,   no_rdwr,   no_rdwr,  no_ioctl  },
  /* 4: /dev/mem etc	System devices (one offs) */
  {  no_open,      no_close,    sys_read, sys_write, sys_ioctl  },
  /* Pack to 7 with nxio if adding private devices and start at 8 */
};

bool validdev(uint16_t dev)
{
    /* This is a bit uglier than needed but the right hand side is
       a constant this way */
    if(dev > ((sizeof(dev_tab)/sizeof(struct devsw)) << 8) - 1)
	return false;
    else
        return true;
}

/* Stuff for bringing up our own IRQ handling */

unsigned int exception_handler(struct exception_frame *ef, uint32_t cause)
{
	di();
	uint32_t vaddr;
	uint_fast8_t s = 0;
	asm volatile ("rsr.excvaddr %0" : "=a" (vaddr));

	kprintf("%d @ %p %p\n", cause, vaddr, ef);
	kprintf("FATAL EXCEPTION %d @ %p with %p(%p):\n", cause, ef->epc1, vaddr, ef->excvaddr);
	kprintf("   a0=%p  sp=%p  a2=%p  a3=%p\n", ef->a0, ef + 1, ef->a2, ef->a3);
	kprintf("   a4=%p  a5=%p  a6=%p  a7=%p\n", ef->a4, ef->a5, ef->a6, ef->a7);
	kprintf("   a8=%p  a9=%p a10=%p a11=%p\n", ef->a8, ef->a9, ef->a10, ef->a11);
	kprintf("  a12=%p a13=%p a14=%p a15=%p\n", ef->a12, ef->a13, ef->a14, ef->a15);

	if (udata.u_insys)
		panic("fatal");
	/*
	 *	Synchronous user exception
	 */
	switch (cause) {
		case 0:	/* Illegal instruction usually */
			s = SIGILL;
			break;
		case 2:	/* Physical address/data fetch/store error - instruction */
		case 3: /* Physical address/data fetch/store error - data */
			s = SIGSEGV;
			break;
		case 6:	/* Divide by zero */
			s = SIGFPE;
			break;
		case 9:	/* Alignment trap */
			s = SIGBUS;
			break;
		case 20: /* Instruction load from data space */
			s = SIGSEGV;
			break;
		case 28: /* Invalid address traps */
		case 29:
			s = SIGSEGV;
			break;
		case 5:	/* Window trap - can't happen */
		case 8:	/* Privilege trap - can't happen */
		default: /* Should not happen */
			panic("badex");
			break;
	}

	/* Requeue any asynchronous pending signal */
	recalc_cursig();
	return s;
}


static void timer_isr(void)
{
	const uint32_t clocks_per_tick = (CPU_CLOCK*1000000) / TICKSPERSEC;
	uint32_t ccount;

	asm volatile ("rsr.ccount %0" : "=a" (ccount));
	asm volatile ("wsr.ccompare0 %0" :: "a" (ccount + clocks_per_tick));
	asm volatile ("esync");

	timer_interrupt();
}

void device_init(void)
{
	flash_dev_init();
	sd_rawinit();
	devsd_init();
	ets_isr_unmask(1<<ETS_CCOMPARE0_INUM);
}

/*
 *	Interrupts
 *	0: WDEV FIQ
 *	1: SLC
 *	2: SPI
 *	3: RTC
 *	4: GPIO
 *	5: UART
 *	6: TICK
 *	7: SOFT
 *	8: WDT
 *	9: FRC1
 *	10: FRC2
 */

static void irq_clear(uint32_t irq)
{
	asm volatile ("wsr.intclear %0; esync" :: "a" (irq));
}

void interrupt_handler(uint32_t interrupt)
{
	if (interrupt & (1 << 6)) {
		timer_isr();
		irq_clear(1 << 6);
	}
	if (interrupt & (1 << 5)) {
		tty_interrupt();
		irq_clear(1 << 5);
	}
}
