#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "defs.h"
#include "data.h"

#define MAX_DEFER	64

static off_t offset_stack[MAX_DEFER];
static uint8_t next_defer;
static int defer;

/* Stack chunks of assembler output to be pasted back into the file later.
   We use this to put arguments in the right order while not having to use
   tons of RAM */

void defer_output(void)
{
    oflush();
    if (next_defer == MAX_DEFER)
        error("too many defers");
    /* Remember where our block started */
    offset_stack[next_defer++] = lseek(defer, 0L, 1);
    output = defer;
}

static void copy_block(void)
{
    /* obuf is free at this point */
    long p = offset_stack[next_defer];
    long size = lseek(defer, 0L, 1) - p;
    lseek(defer, p, 0);
    
    while (size) {
        /* Can this ever really be over 64K ?? */
        unsigned int n = size;
        if (n > 512)
            n = 512;
        if (read(defer, obuf, n) != n)
         error("rd");
        if (write(target, obuf, n) !=n)
         error("wr");
        size -= n;
    }
    lseek(defer, p, 0);
}

void end_defer(void)
{
    --next_defer;
    /* FIXME: we should fastpath the case of all the bits being in
       the buffer - in which case we can just fire it at target */
    oflush();
    /* Copy from this position to the current position into the target
       file */
    copy_block();
    /* We are back into the main flow, stop outputting via the defer file */
    if (next_defer == 0)
        output = target;
}

void defer_init(void)
{
    defer = open(".scc-spool", O_RDWR|O_CREAT|O_TRUNC, 0600);
    if (defer == -1) {
        error("tmp file open failed");
        exit(1);
    }
    unlink("scc-spool");
}
