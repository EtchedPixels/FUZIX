#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#include <features.h>
#include <fcntl.h>

#ifndef L_SET

#define L_SET           0       /* absolute offset */
#define L_INCR          1       /* relative to current offset */
#define L_XTND          2       /* relative to end of file */

#endif

#endif
