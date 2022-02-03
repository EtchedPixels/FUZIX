#include <unistd.h>

int fsync(int fd)
{
    sync();
    return 0;
}
