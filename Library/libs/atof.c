/* Copyright (C) Robert de Bath <robert@debath.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <stdlib.h>

double atof(const char *p)
{
   return strtod(p, (char**)0);
}
