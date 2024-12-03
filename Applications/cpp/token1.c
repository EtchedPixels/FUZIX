
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cc.h"

#ifdef __GNUC__
__inline
#endif
static unsigned int hash1(register const char *, register unsigned int);

#include "token1.h"
