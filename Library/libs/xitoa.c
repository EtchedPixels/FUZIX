/* numeric/string conversions package
 */

#include <stdlib.h>

/*********************** xitoa.c ***************************/
char *_itoa(int i) {
    return ltostr((long) i, 10);
}
