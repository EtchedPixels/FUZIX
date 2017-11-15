/*      File error.c: 2.1 (83/03/20,16:02:00) */
/*% cc -O -c %
 *
 */

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "data.h"

void errchar(char c)
{
        write(2, &c, 1);
}

void error(char *ptr)
{
        int k = 0;;

        write(2, line, strlen(line));
        errchar('\n');
        while (k < lptr) {
                if (line[k] == 9)
                        errchar('\t');
                else
                        errchar(' ');
                k++;
        }
        errchar('^');
        errchar('\n');
        write(2, ptr, strlen(ptr));
        errchar('\n');
        errcnt++;
}
