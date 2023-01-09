#include <kernel.h>
#include <input.h>
#include <devinput.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include <printf.h>

/* Some platforms polling the mouse is expensive so let them query */
uint_fast8_t ps2m_open;

static uint_fast8_t open;
static uint_fast8_t fivebutton;

/* PS/2 mouse - rather simpler than the keyboardc */

#define SEND 0x100
#define FAIL 0x200
#define STOP 0xFFFF

static const uint16_t mouse_init[] = {
    SEND|0xFF,
    FAIL|0xFA,
    FAIL|0xAA,
    FAIL|0x00,
    SEND|0xF3,
    FAIL|0xFA,
    STOP
    /* May bee followed by an FA or FE */
};

static const uint16_t mouse_scrolltest[] = {
    SEND|0xF3,			/* Set sample rate */
    FAIL|0xFA,			/* Magic handshake is to 200,100,80 */
    SEND|0xC8,
    FAIL|0xFA,
    SEND|0xF3,
    FAIL|0xFA,
    SEND|0x64,
    FAIL|0xFA,
    SEND|0xF3,
    FAIL|0xFA,
    SEND|0x50,
    FAIL|0xFA,
    SEND|0xF2,			/* Should now reply to get ID */
    FAIL|0xFA,
    FAIL|0x03,			/* 0x03 is intellimouse protocol */
    STOP
};

static const uint16_t mouse_fivetest[] = {
    SEND|0xF3,			/* Set sample rate */
    FAIL|0xFA,			/* Magic handshake is to 200,200,80 */
    SEND|0xC8,
    FAIL|0xFA,
    SEND|0xF3,
    FAIL|0xFA,
    SEND|0x64,
    FAIL|0xFA,
    SEND|0xF3,
    FAIL|0xFA,
    SEND|0x64,
    FAIL|0xFA,
    SEND|0xF2,			/* Should now reply to get ID */
    FAIL|0xFA,
    FAIL|0x04,			/* 0x04 is five button intellimouse protocol */
    STOP
};

static const uint16_t mouse_setup[] = {
    SEND|0xE6,			/* Scaling 1:1 */
    FAIL|0xFA,
    SEND|0xF3,			/* 10 samples a second */
    FAIL|0xFA,
    SEND|0x0A,
    FAIL|0xFA,
    STOP
};

static const uint16_t mouse_open[] = {
    SEND|0xF4,
    FAIL|0xFA,
    STOP
};

static const uint16_t mouse_close[] = {
    SEND|0xF5,
    FAIL|0xFA,
    STOP
};

/* One day we might want to handle FE/FC rules */
static unsigned mouse_op(const uint16_t *op)
{
    uint8_t r;
    ps2busy = 1;
    while(*op != STOP) {
        if (*op & SEND) {
            if (ps2mouse_put(*op)) {
                ps2busy = 0;
                return 0;
            }
        } else {
            r = ps2mouse_get();
            if ((*op & FAIL) && r != (*op & 0xFF)) {
                ps2busy = 0;
                return 0;
            }
        }
        op++;
    }
    ps2busy = 0;
    return 1;
}

static uint8_t packet[4];
static uint_fast8_t packc;
static uint_fast8_t packsize = 4;

/* We received a 3 or 4 byte packet. Now process it
   Fudge the movement slightly as the PS/2 mouse is 9bit */
static void ps2mouse_event(void)
{
    static uint8_t event[4];
    /* Event code and button bits */
    event[0] = MOUSE_REL;
    event[1] = packet[0] & 7;
    event[1] = packet[1] >> 1;	/* Scale down and add sign */
    if (packet[0] & 0x10)
        event[1] |= 0x80;
    /* On a classic this byte is always 0 in our buffer so we do the
       right thing */
    event[2] = packet[2] >> 1;
    if (packet[0] & 0x20)
        event[2] |= 0x80;
    event[3] = packet[3] << 5;	/* Only 3 bits are used so scale up */
    /* These bits may have other stuff in them on a non 5 button protocol
       mouse so we check the type */
    if (fivebutton)
        event[1] |= (packet[3] & 0x18);
    /* The rest is up to the platform */
    plt_ps2mouse_event(event);
}

void ps2mouse_byte(uint_fast8_t byte)
{
    packet[packc++] = byte;
    if (packc == packsize) {
        ps2mouse_event();
        packc = 0;
        return;
    }
}

uint_fast8_t ps2mouse_open(void)
{
    open =  mouse_op(mouse_open);
    packc = 0;
    ps2m_open++;
    return open;
}

void ps2mouse_close(void)
{
    open = 0;
    mouse_op(mouse_close);
    ps2m_open--;
}

int ps2mouse_init(void)
{
    unsigned int r;
    uint_fast8_t i;
    uint_fast8_t buttons = 5;

    ps2busy = 1;
    /* We may have FF or FF AA or FF AA 00 or other info queued before
       our reset, if so empty it out */
    for (i = 0; i < 4; i++) {
        if (ps2mouse_get() == PS2_NOCHAR)
            break;
    }

    r = mouse_op(mouse_init);
    if (r == 0)
        return 0;
    /* Intellimouse 4/5 button protocol */
    fivebutton = mouse_op(mouse_fivetest);
    /* Intellimouse protocol (scroll wheel) */
    if (!fivebutton) {
        buttons = 3;
        if (!mouse_op(mouse_scrolltest))
            /* PS/2 clasic */
            packsize = 3;
    }
    /* Set up but don't enable reporting yet */
    mouse_op(mouse_setup);
    return 1;
}
