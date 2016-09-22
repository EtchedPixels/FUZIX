#include <unistd.h>

int fdatasync(int fd)
{
    sync();
}
