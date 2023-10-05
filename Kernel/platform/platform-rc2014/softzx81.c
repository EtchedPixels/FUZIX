#include <kernel.h>
#include <kdata.h>
#include <softzx81.h>
#include <vt.h>
#include <devtty.h>

static ptptr zxproc[5];	/* One per vt */

int softzx81_on(uint_fast8_t minor)
{
    if (zxproc[minor] != NULL) {
        udata.u_error = EBUSY;
        return -1;
    }
    zxproc[minor] = udata.u_ptab;
    soft81_on++;
    return 0;
}

int softzx81_off(uint_fast8_t minor)
{
    if (zxproc[minor] == NULL) {
        udata.u_error = EINVAL;
        return -1;
    }
    zxproc[minor] = NULL;
    /* Fix up the video and font */
    tms9918a_reset();
    tms9918a_reload();
    soft81_on--;
    return 0;
}

/* Kill anything that execs or terminates */
void plt_udma_kill(ptptr proc)
{
    uint_fast8_t minor;
    for (minor = 1; minor <= 4; minor ++) {
        if (zxproc[minor] == proc)
            softzx81_off(minor);
    }
}

void plt_udma_sync(ptptr p)
{
}

void softzx81_int(void)
{
    uint_fast8_t n = inputtty;
    ptptr p;

    if (inputtty <= 4) {
        p  = zxproc[inputtty];
        if (p != NULL)
            soft_zx81(p);
    }
}
