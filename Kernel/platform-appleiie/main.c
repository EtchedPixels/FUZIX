#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <devhd.h>
#include <apple.h>

uint8_t kernel_flag = 1;
uint8_t cols = 40;
uint8_t card_present;
uint8_t prodos_slot;
uint8_t pascal_slot;
uint8_t model;

void plt_idle(void)
{
    irqflags_t flags = di();
    tty_poll();
    irqrestore(flags);
}

void do_beep(void)
{
    /* Strobe speaker bit - TODO */
}


/* Below wants to end up mostly in discard */

#ifdef CONFIG_RAMWORKS
/*
 * Map handling: We have flexible paging. Each map table consists of a set of pages
 * with the last page repeated to fill any holes.
 */

void pagemap_init(void)
{
    int i;
    /* Bank 0 is the kernel */
    for (i = 15 ; i > 0; i--)
        pagemap_add(i * 8);
}

#endif

void map_init(void)
{
}

extern int strcmp(const char *, const char *);

uint8_t plt_param(char *p)
{
    if (strcmp(p,"40") == 0) {
        cols = 40;
        return 1;
    }
    if (strcmp(p,"80") == 0) {
        cols = 80;
        return 1;
    }
    return 0;
}

static void unsupported(void)
{
    kputs(" (unsupported) ");
}

/* Make sure all the strings in this lot end up in discard */
static const char *classid[] = {
    "printer",
    "joystick",
    "serial/parallel",
    "modem",
    "sound/speech",
    "clock",
    "storage",
    "80 column card",
    "network/bus",
    "special"
};

static void pascal_card(uint8_t slot, uint8_t cid)
{
    uint8_t class = cid >> 4;
    if (class == 0 || class > 10) {
        kputs("unknown");
        return;
    }
    kputs(classid[class - 1]);
    switch(class) {
#if 0
    case 1:
        pascal_printer_install(slot, cid);
        break;
    case 2:
        joystick_install(slot, cid);
        break;
    case 3:
        /* 31 is superserial etc */
        if (cid == 0x31)
            superserial_install(slot);
        else
            pascal_serial_install(slot, cid);
        break;
    case 4:
        pascal_serial_install(slot, cid);
        break;
    case 5:
        audio_install(slot, cid);
        break;
    case 6:
        clock_install(slot, cid);
        break;
    case 8:
        break;
    case 9:
        network_install(slot, cid);
        break;
    case 10:
        special_install(slot, cid);
        break;
#endif
    case 7:
        hd_install(slot);
        break;
    default:
        unsupported();
    }
}

/* Slot detection on Apple is a bit magic. It started out as sneaky ROM
   peeking and evolved later into proper protocols */

void scan_slots(void)
{
    uint8_t i = 1;	/* slot 0 is the motherboard */
    volatile uint8_t *card = (volatile uint8_t *)0xC100;
    
    for (i = 1; i < 8; i++, card += 0x100) {
        kprintf("\n%c - ", '0' + i);
        if (!(card_present & (1 << i))) {
            kputs("empty");
            continue;
        }
        /* Pascal and also Serial cards: from dumb to dumber if they don't
           have the Pascal interface present */
        if (card[5] == 0x38 && card[7] == 0x18) {
            if (card[11] == 0x01) {
                /* This slot has a pascal (aka Protocol Convertor) attached.
                   In other words basically we've got device driver firmware
                   that does all the work for us, and we have a card id for
                   type/device */
                pascal_slot |= (1 << i);
                pascal_card(i, card[12]);
            } else {
                kputs("serial");
                unsupported();
            }
            continue;
        }
        /* This is how ProDOS finds disk driver interfaces. The ProDOS
           interface isn't as nice as the Pascal one */
        if (card[1] == 0x20 && card[3] == 0x00 && card[5] == 0x03) {
            /* storage */
            if (card[255] == 0xFF) {
                /* Old ROM floppy drive: needs custom driver */
                kputs("13 sector/track floppy");
                unsupported();
            } else if (card[255] == 0x00) {
                /* New ROM floppy drive: needs custom driver */
                kputs("16 sector/track floppy");
                unsupported();
            } else {
                /* We use the firmware interface */
                kputs("storage");
                prodos_slot |= (1 << i);
                hd_install(i);
            }
            continue;
        }
        if (card[1] == 0xB0 && card[2] == 0x20) {
            kputs("tablet");
            unsupported();
            continue;
        }
        /* This one is important, we must find it to get a period timer */
        if (card[5] == 0x38 && card[7] == 0x18 && card[11] == 0x01 &&
            card[12] == 0x20 && card[0xFB] == 0xD6) {
            kputs("mouse");
            continue;
        }
        /* This is how ProDOS recognizes a clock card */
        if (card[0] == 0x08 && card[2] == 0x28 && card[4] == 0x58 &&
            card[6] == 0x70) {
            kputs("clock");
            continue;
        }
        /* TODO cards with pascal idents not in the above */
        kputs("unknown");
    }
    kputs("\n\n");
}

/* Process the ProDOS bits */
void breadcrumbs(void)
{
    uint8_t *dos = (uint8_t *)0xBF00;
    uint8_t bits = dos[0x98];

    card_present = dos[0x99];

    if ((bits & 0x30) != 0x30)
        panic("128K memory required");
    if ((bits & 0x34) == 0x20)
        model = APPLE_IIE;
    if ((bits & 0x34) == 0x24)
        model = APPLE_IIC;
    /* IIgs is rather different */
    if (model == APPLE_UNKNOWN)
        panic("Apple IIe or IIc required");
}
