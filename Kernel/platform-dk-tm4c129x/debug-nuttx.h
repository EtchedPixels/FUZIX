#ifndef __DEBUG_NUTTX_H
#define __DEBUG_NUTTX_H

#define DEBUGPANIC() for (;;) { }
#define DEBUGASSERT(x) if (!(x)) for (;;) { }
#define gpioinfo(...)

#endif /* __DEBUG_NUTTX_H */
