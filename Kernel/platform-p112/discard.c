#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <z180.h>
#include "config.h"
#ifdef CONFIG_P112_FLOPPY
#include "devfd.h"
#endif

void pagemap_init(void)
{
    int i;

    /* P112 has RAM across the full physical 1MB address space
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x10; i < 0x100; i+=0x10){
        pagemap_add(i);
    }
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_process(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

uint16_t bootdevice(unsigned char *s)
{
    unsigned int b = 0, n = 0;
    unsigned char c;
    bool ok = false;

    /* skip spaces */
    while(*s == ' ')
        s++;

    /* we're expecting one of;
     * hdX<num>
     * fd<num>
     * <num>
     */


    switch(*s | 32){
        case 'f': /* fd */
            b += 256;
        case 'h': /* hd */
            s++;
            ok = true;
            if((*s | 32) != 'd')
                return -1;
            s++;
            if(b == 0){ /* hdX */
                c = (*s | 32) - 'a';
                if(c & 0xF0)
                    return -1;
                b += c << 4;
                s++;
            }
            break;
        default:
            break;
    }

    while(*s >= '0' && *s <= '9'){
        n = (n*10) + (*s - '0');
        s++;
        ok = true;
    }

    if(ok)
        return (b + n);
    else
        return -1;
}
