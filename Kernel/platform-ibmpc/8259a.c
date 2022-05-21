#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <ibmpc.h>

/*
 *	8259A interrupt controllers
 *
 *	Implement the interrupt logic for classic PC, PC/XT and PC/AT
 *	platforms.
 */
 
static irqvector_t irqvec[16];
static uint8_t irq_mask[2];
static uint8_t irq_port[2] = { 0x21, 0xA1 };

void mask_irq(uint8_t irq)
{
    irqflags_t mask = di();
    uint8_t chip = irq >> 3;
    irq &= 7;
    irq_mask[chip] |= (1 << irq);
    outb(irq_mask[chip], irq_port[chip]);
    irqrestore(mask);
}

void unmask_irq(uint8_t irq)
{
    irqflags_t mask = di();
    uint8_t chip = irq >> 3;
    irq &= 7;
    irq_mask[chip] &= ~(1 << irq);
    outb(irq_mask[chip], irq_port[chip]);
    irqrestore(mask);
}

int8_t int_to_irq(uint8_t irq)
{
    if (irq > 15)
        return -EINVAL;
    if (irq > 7 && pc_at == 0)
        return -EINVAL;
    if (irq == 2 && pc_at == 1)
        irq = 9;
    return irq;
}

void init_irq(void)
{
    if (inb(0xA1) == 0xFF)
        pc_at = 0;
}

int set_irq(int8_t i, irqvector_t handler)
{
    irqflags_t mask = di();
    i = int_to_irq(i);
    if (i < 0)
        return i;
    if (irqvec[i])
        return -EBUSY;
    /*
     *	Replace the existing vector with the right irq{n} handler from our
     *	table. We don't replace them at start up as we are mostly a BIOS user
     */
//FIXME    hook_irqvec(i);
    irqvec[i] = handler;
    unmask_irq(i);
    irqrestore(mask);
    return 0;
}

void do_plt_interrupt(int16_t irq)
{
    (irqvec[irq])(irq);
}
