#include <unistd.h>

int fdatasync(int fd)
{
    sync();
    return 0;
}
