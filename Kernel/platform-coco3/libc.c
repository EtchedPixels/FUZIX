#include "cpu.h"

int strcmp( char *a, char *b ){
    return strncmp(a, b, strlen(a));
}

int strncmp( char *a, char *b, int n){
    int ret;
    while (n--){
	ret = *a - *b++;
	if( ! *a || ret )
	    return ret;
	a++;
    }
    return 0;
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
