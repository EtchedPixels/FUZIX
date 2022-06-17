/*
 *	Driver for an actual PS/2 style keyboard controller
 *
 *	FIXME: we assumed we can talk reliably to mouse only or kbd only
 *	that will need changes in the layers above to remove.
 */

#include <kernel.h>
#include <printf.h>
#include <ps2kbd.h>
#include <ps2mouse.h>

__sfr __at 0x8C	ps2stat;
__sfr __at 0x8D ps2data;

#define TIMEOUT		0
#define TIMEOUT_PUT	0

static int ps2_present;

static int ps2cmd(uint8_t v)
{
    uint16_t t = TIMEOUT_PUT;

    while(--t &&  (ps2stat & 2));
    ps2stat = v;
    if (t == 0)
        return 1;
    return 0;
}

static void ps2put(uint8_t v)
{
    uint16_t t = TIMEOUT_PUT;

    while(--t && (ps2stat & 2));
    ps2data = v;
    if (t == 0)
        kprintf("ps2put: timeout\n");
}

static int ps2get(void)
{
    uint16_t t = TIMEOUT;
    uint8_t v;

    while(++t && !(ps2stat & 1));
    if (t == 0)
        return -1;
    v = ps2data;
    return v;
}

int ps2kbd_put(uint_fast8_t c)
{
    ps2put(c);
    while ((ps2stat & 0xC2) == 2);
    if (ps2stat & 0xC0)
        return -1;
    return 0;
}

int ps2mouse_put(uint_fast8_t c)
{
    uint8_t stat;
    ps2cmd(0xD4);
    ps2put(c);
    /* Wait for it to go */
    /* TIMEOUT ? */
    while (((stat = ps2stat) & 0x02) == 2);
#if 0    
    if (stat & 0xC0) {
        kprintf("mouse timeout %2x\n", stat);
        return -1;
    }
#endif    
    return 0;
}

/*
 *	Be very careful how we handle status. As far as I can tell the
 *	behaviour of the VT82C42 is
 *
 *	0x20	-	next byte is PS/2
 *	0x01	-	next byte is Keyboard
 *
 *	Reading an 0x20 status seems to clear it so we need to be careful
 *	to not re-read the status but act on the bits received
 */
void ps2_int(void)
{
    uint8_t data;
    uint8_t stat;
    static int count;

    if (!ps2_present)
        return;
        
    while ((stat = ps2stat) & 0x21) {
        data = ps2data;
        if (stat & 0x20)
            ps2mouse_byte(ps2data);
        else 
            ps2kbd_byte(data);
    }
}

/* These are only used during init. Therefore it's okay to throw away
   any random bits of conversation from the other device. Maybe we should
   disable them and have a core ps2 hook for turning devices on/off where
   relevant ?? FIXME */

unsigned int ps2kbd_get(void)
{
    uint8_t c;
    uint16_t t = TIMEOUT;
        
    do {
        c = ps2stat;
        if ((c & 0xE1) == 0x01)
            return ps2data;
        /* Random mouse chatter */
        if (c & 0x20)
            ps2data;
    } while (!(c & 0xC0) && --t);
    return -1;
}

unsigned int ps2mouse_get(void)
{
    uint8_t c;
    uint16_t t = TIMEOUT;
    
    do {
        c = ps2stat;
        if ((c & 0x20) == 0x20) {
            c = ps2data;
            return c;
        }
        /* Random keyboard chatter */
        if (c & 0x01) {
            ps2data;
        }
    } while (--t);
    return -1;
}

int ps2port_init(void)
{
    uint8_t st;
    uint8_t mouse;
    irqflags_t irq = di();
    /* Disable the interfaces */
    /* If it times out there isn't a PS/2 port at all */
    if (ps2cmd(0xAD))
        return 0;
    ps2cmd(0xA7);
    /* Self test */
    ps2cmd(0xAA);
    if ((st = ps2get()) != 0x55) {
        kprintf("PS/2 self test: %2x\n", st);
        irqrestore(irq);
        return 0;
    }
    kputs("PS/2 port at 0x8C [ ");
    ps2_present = 1;

    /* Check the ports are working */
    ps2cmd(0xAB);
    if (ps2get() == 0)
        kputs("Keyboard ");

    ps2cmd(0xA9);
    if ((mouse = ps2get()) == 0)
        kputs("Mouse ");
    else
        ps2cmd(0xA7);
    kputs("]\n");

    /* Set configuration */
    ps2cmd(0x60);
    ps2put(0x0F);	/* Ports on, interrupts on, translation off, no lock */

    /* Turn the interfaces back on */
    ps2cmd(0xAE);
    if (!mouse)
        ps2cmd(0xA8);
    irqrestore(irq);
    return 1;
}

void ps2kbd_beep(void)
{
}
