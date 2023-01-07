#ifndef _KMOD_H
#define _KMOD_H

struct kmodaddr {
    uint8_t *start;
    uint8_t *end;
};

struct kmodinfo {
    uint8_t *end;	/* Expected end address */
    usize_t size;	/* Size in bytes */
    usize_t discard;	/* Bytes to discard after init run */
};

typedef int (*modfunc)(void *);

#define MODIOC_SPACE	0x47D0
#define MODIOC_LOAD	0x47D1

extern int kmod_ioctl(uarg_t request, char *data);
extern void kmod_init(void *start, void *end);

extern unsigned plt_kmod_set(uint8_t *top);

#endif
