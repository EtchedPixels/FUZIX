/* numeric/string conversions package
 */

#include <stdlib.h>

/**************************** atoi.c ****************************/
int atoi(char *str) {
    return (int) strtol(str, NULL, 10);
}

