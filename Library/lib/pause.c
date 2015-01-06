#include <unistd.h>

int pause(void)
{
  return _pause(0);
}
