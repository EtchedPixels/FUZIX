/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

extern char ** environ;
#define ADD_NUM 4

int putenv(char *var)
{
static char ** mall_env;
static int extras;
   char **p, **d;
   char * r;
   int len;

   r = strchr(var, '=');
   if( r == 0 )  len = strlen(var);
   else          len = r-var;

   for(p=environ; p && *p; p++)
   {
      if( memcmp(var, *p, len) == 0 && (*p)[len] == '=' )
      {
         while((p[0] = p[1]) != NULL ) p++;
         extras++;
         break;
      }
   }
   if( r == 0 ) return 0;
   if( extras <= 0 )	/* Need more space */
   {
      d = malloc((p-environ+1+ADD_NUM)*sizeof(char*));
      if( d == 0 ) return -1;

      memcpy((void*) d, (void*) environ, (p-environ+1)*sizeof(char*));
      p = d + (p-environ);
      extras=ADD_NUM;

      if( mall_env ) free(mall_env);
      environ = d;
      mall_env = d;
   }
   *p++ = var;
   *p = '\0';
   extras--;

   return 0;
}


