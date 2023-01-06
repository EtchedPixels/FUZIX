#include <kernel.h>
#include <kdata.h>
#include <kmod.h>

#ifdef CONFIG_KMOD

/* A basic implementation of the module loader interface. This version
   should do for most simple setups but things like flat memory will need
   their own version and mechanism using the actual kernel allocators */

struct kmodaddr kmod_space;

int kmod_ioctl(uarg_t request, char *data)
{
    struct kmodinfo m;
    uint8_t *p;

    switch(request) {
    case MODIOC_SPACE:
        return uput(&kmod_space, data, sizeof(kmod_space));
    case MODIOC_LOAD:
        if (uget(data, &m, sizeof(m)) < 0)
            return -1;
        if (kmod_space.end - kmod_space.start < m.size) {
            udata.u_error = ENOSPC;
            return -1;
        }
        if (m.end != kmod_space.end) {
            udata.u_error = EAGAIN;
            return -1;
        }
        /* It worked, commit the space and load */
        data += sizeof(m);
        if (valaddr(data, m.size) != m.size) {
            udata.u_error = EFAULT;
            return -1;
        }
        /* Reclaim buffers etc if needed */
        if (plt_kmod_set(kmod_space.end - m.size))
            return -1;
        kmod_space.end -= m.size;
        p = kmod_space.end;
        /* This is a hack. uget() on some systems is optimized to know that
           kernel code never overlaps common space. Force ugetc usage so that
           we don't hit that. This makes our loader a shade slower but this
           is hardly a performance path */
        while(m.size--)
            *p++ = ugetc(data++);
        /* Run the module initialization */
        ((modfunc)(kmod_space.end))(kmod_space.end);
        /* And move back up for the discarded start area if any */
        kmod_space.end += m.discard;
        return 0;
    }
    return -1;
}

void kmod_init(void *start, void *end)
{
    kmod_space.start = start;
    kmod_space.end = end;
}
#endif
