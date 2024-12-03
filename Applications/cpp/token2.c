
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cc.h"

#ifdef __GNUC__
__inline
#endif
static unsigned int hash2(register const char *, register unsigned int);

#include "token2.h"
