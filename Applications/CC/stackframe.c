/*
 *	Assign offsets in the stack frame (or elsewhere)
 */

#include <stdio.h>
#include "compiler.h"

static unsigned arg_frame;
static unsigned local_frame;
static unsigned local_max;

/*
 *	We will need to deal with alignment rules later
 */

unsigned alloc_room(unsigned *p, unsigned type, unsigned storage)
{
    unsigned s = type_sizeof(type);
    unsigned a = target_alignof(type, storage);

    *p = (*p + a - 1) & ~(a - 1);
    a = *p;
    *p += s;
    return a;
}

unsigned assign_storage(unsigned type, unsigned storage)
{
    unsigned *p = &arg_frame;
    unsigned n;
    if (storage == S_AUTO)
        p = &local_frame;
    n = alloc_room(p, type, storage);
    if (storage == S_AUTO) {
        if (local_frame > local_max)
            local_max = local_frame;
    }
    return n;
}

void mark_storage(unsigned *a, unsigned *b)
{
    *a = arg_frame;
    *b = local_frame;
}

void pop_storage(unsigned *a, unsigned *b)
{
    arg_frame = *a;
    local_frame = *b;
}

void init_storage(void)
{
    arg_frame = 0;
    local_frame = 0;
    local_max = 0;
}

unsigned frame_size(void)
{
    return local_max;
}

unsigned arg_size(void)
{
    return arg_frame;
}
