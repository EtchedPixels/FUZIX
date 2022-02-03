/*
 * This file lifted in toto from 'Dlibs' on the atari ST  (RdeBath)
 *
 * 
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 */

#include <stdio.h>
#include <string.h>

void *lfind(const void *key, const void *base,
	    size_t *num, size_t size,
	    int (*cmp)(const void *,const void *))
{
   register size_t n = *num;

   while (n--)
   {
      if ((*cmp) (base, key) == 0)
	 return (void *)base;
      base = (char *)base + size;
   }
   return NULL;
}

void *lsearch(const void *key, void *base,
	      size_t *num, size_t size,
	      int (*cmp)(const void *, const void *))
{
   void *p;

   if ((p = lfind(key, base, num, size, cmp)) == NULL)
   {
      p = memcpy(((char *)base + (size * (*num))), key, size);
      ++(*num);
   }
   return p;
}
