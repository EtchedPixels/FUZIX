#include <unistd.h>
#include <stdlib.h>

void cfree(void *ptr, size_t nelem, size_t elsize)
{
  free(ptr);
}
