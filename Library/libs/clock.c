/*************************** CLOCK ************************************/

#include <types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/times.h>

/* FIXME: CLOCKS_PER_SEC query */

clock_t clock(void) {
    struct tms __tms;
    times(&__tms);
    return (__tms.tms_utime * CLOCKS_PER_SEC);
}


