#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <vt.h>
#include <ps2kbd.h>
#include <ps2mouse.h>
#include <vdp1.h>
#include "config.h"
#include <z180.h>

extern uint8_t far_read(uint8_t page) __fastcall;
extern uint8_t far_write(uint8_t page) __fastcall;
extern uint8_t far_writable(uint8_t page) __fastcall;

static uint16_t low_mem = 128;
static uint16_t high_mem = 0;

uint8_t fontdata_6x8;		/* Dummy for linker */

void init_hardware_c(void)
{
    /* Bring up the video so we can see what is going on */
    vdp_init();
    vdp_wipe_consoles();
/*    vdp_load_font(); - loaded by boot loader */
    vdp_restore_font();
    vtinit();
    vdp_reload();
    ps2kbd_init();
    ps2mouse_init();

    /* Label memory (we are using bank 0 for kernel) */
    far_write(1);
    far_write(3);	/* 128K above */
    far_write(8);	/* Second bank */
    far_write(11);	/* 128K above */
    /* If we have 512K low we will see 1,3. If we have 128K low we will see 3,3 */
    /* If we have 512K high we will see 8,11 if not 11,11. We also need to test
       it is writable space */
    if (far_read(1) == 1)
        low_mem = 512;
    if (far_writable(8)) {
        if (far_read(8) == 4)
            high_mem = 128;
        else
            high_mem = 512;
    }
    ramsize = low_mem + high_mem;
    if (ramsize == 128)
        panic("not enough RAM");
    procmem = ramsize - 64;
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    uint8_t i;
    uint8_t n = low_mem / 64;

    for (i = 1; i < n; i++)
        pagemap_add(i << 4);

    n = 8 + high_mem / 64;
    for (i = 8; i < n; i++)
        pagemap_add(i << 4);
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_proc(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}
