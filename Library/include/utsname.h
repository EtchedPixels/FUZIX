#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H
#ifndef __TYPES_H
#include <types.h>
#endif

struct utsname {
<<<<<<< HEAD
    char sysname[14];
    char nodename[14];
    char release[8];
    char version[8];
    char machine[8];
    char domainname[14];
=======
	char sysname[14];
	char nodename[14];
	char release[8];
	char version[8];
	char machine[8];
	char domainname[14];
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91
};

extern int uname __P ((struct utsname * __utsbuf));

#endif

