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

//void syscall_handler(struct __exception_frame* ef, int cause)
//{
//	udata.u_callno = ef->a2;
//	udata.u_argn = ef->a3;
//	udata.u_argn1 = ef->a4;
//	udata.u_argn2 = ef->a5;
//	udata.u_argn3 = ef->a6;
//	udata.u_insys = 1;
//
//	unix_syscall();
//
//	udata.u_insys = 0;
//
//	ef->a2 = udata.u_retval;
//	ef->a3 = udata.u_error;
//
//	ef->epc += 3; /* skip SYSCALL instruction */
//}
//
//static void fatal_exception_cb(struct __exception_frame* ef, int cause)
//{
//	di();
//	uint32_t vaddr;
//	asm volatile ("rsr.excvaddr %0" : "=a" (vaddr));
//
//	kprintf("FATAL EXCEPTION %d @ %p with %p:\n", cause, ef->epc, vaddr);
//	kprintf("   a0=%p  sp=%p  a2=%p  a3=%p\n", ef->a0, ef->sp, ef->a2, ef->a3);
//	kprintf("   a4=%p  a5=%p  a6=%p  a7=%p\n", ef->a4, ef->a5, ef->a6, ef->a7);
//	kprintf("   a8=%p  a9=%p a10=%p a11=%p\n", ef->a8, ef->a9, ef->a10, ef->a11);
//	kprintf("  a12=%p a13=%p a14=%p a15=%p\n", ef->a12, ef->a13, ef->a14, ef->a15);
//	for (;;)
//		;
//}

//static void timer_isr(void* user, struct __exception_frame* ef)
//{
//	const uint32_t clocks_per_tick = (CPU_CLOCK*1000000) / TICKSPERSEC;
//	irqflags_t irq = di();
//
//	uint32_t ccount;
//	asm volatile ("rsr.ccount %0" : "=a" (ccount));
//
//	asm volatile ("wsr.ccompare0 %0" :: "a" (ccount + clocks_per_tick));
//	asm volatile ("esync");
//
//	timer_interrupt();
//
//	irqrestore(irq);
//}

void device_init(void)
{
//	flash_dev_init();
//	sd_rawinit();
//	devsd_init();
//
//	static const uint8_t fatal_exceptions[] =
//		{ 0, 2, 3, 5, 6, 8, 9, 20, 28, 29 };
//	for (int i=0; i<sizeof(fatal_exceptions)/sizeof(*fatal_exceptions); i++)
//		_xtos_set_exception_handler(fatal_exceptions[i], fatal_exception_cb);
//
//	extern fn_c_exception_handler_t syscall_handler_trampoline;
//	_xtos_set_exception_handler(1, syscall_handler_trampoline);
//
//	ets_isr_attach(ETS_CCOMPARE0_INUM, timer_isr, NULL);
//	ets_isr_unmask(1<<ETS_CCOMPARE0_INUM);
}

/* vim: sw=4 ts=4 et: */

