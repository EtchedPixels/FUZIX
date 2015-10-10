#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <device.h>
#include <devtty.h>
#include <carts.h>

uint8_t membanks;
uint8_t system_id;
uint8_t cartslots = 1;
uint8_t carttype[4];
uint16_t cartaddr[4];
uint8_t bootslot = 0;

void platform_idle(void)
{
}

void do_beep(void)
{
}


