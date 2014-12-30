#ifndef _FUZIX_H
#define _FUZIX_H
#include <stdlib.h>
#include <sys/types.h>

/*
 *	FUZIX constants
 */	 

#define __MAXPID 32000

/* uadmin */
#define A_SHUTDOWN		1
#define A_REBOOT		2
#define A_DUMP			3
#define A_FREEZE		4	/* Unimplemented, want for NC100 */
#define A_SWAPCTL		16	/* Unimplemented */
#define A_CONFIG		17	/* Unimplemented */
#define A_FTRACE		18	/* Unimplemented: 
                                          Hook to the syscall trace debug */
#define AD_NOSYNC		1	/* Unimplemented */

#define FILENAME_LEN    30
#define DIR_LEN         32
typedef struct direct {
    uint16_t   d_ino;
    char     d_name[FILENAME_LEN];
} direct;

#endif
