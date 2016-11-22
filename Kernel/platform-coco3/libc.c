#include "cpu.h"

int strcmp( char *a, char *b ){
    int ret;
    while (1){
	ret = *a - *b++;
	if( ! *a || ret )
	    return ret;
	a++;
    }
}

size_t strlen(const char *p)
{
  const char *e = p;
  while(*e++);
  return e-p-1;
}

/* Until we pull out the bits of libgcc that are useful instead */
void abort(void)
{
  while(1);
}

void *malloc(size_t size)
{
  return 0;
}
