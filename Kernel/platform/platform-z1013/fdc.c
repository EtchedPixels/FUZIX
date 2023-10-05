#include <config.h>

static void dummy(void) __naked {
#ifdef CONFIG_FDC765
     __asm
#include "fdc765.s"
    __endasm;
#endif    
}
