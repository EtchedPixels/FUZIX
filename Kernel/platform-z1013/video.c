#include <config.h>

static void dummy(void) __naked {
#ifdef CONFIG_VIDEO_POPPE
     __asm
#include "video-poppe.s"
    __endasm;
#else
     __asm
#include "video-32x32.s"
    __endasm;
#endif    
}
