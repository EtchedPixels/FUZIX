#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>
#include "picosdk.h"

struct u_data udata;
uint8_t progbase[PROGSIZE];
uint8_t sys_cpu = A_ARM;
uint8_t sys_cpu_feat = AF_CORTEX_M0;
uint8_t need_resched;
uaddr_t ramtop = (uaddr_t) PROGTOP;
uint8_t sys_stubs[sizeof(struct exec)];

usize_t valaddr(const uint8_t *base, usize_t size)
{
        if (base + size < base)
                size = MAXUSIZE - (usize_t)base + 1;
        if (!base || base < (const uint8_t *)PROGBASE)
                size = 0;
        else if (base + size > (const uint8_t *)(size_t)udata.u_top)
                size = (uint8_t *)(size_t)udata.u_top - base;
        if (size == 0)
                udata.u_error = EFAULT;
        return size;
}


/* vim: sw=4 ts=4 et: */


