#include <unistd.h>

int swapon(int fd, uint16_t n)
{
    uint16_t sv[3];
    sv[0] = fd;
    sv[1] = n;
    sv[2] = 0;	/* Future use */
    return uadmin(A_SWAPCTL, A_SC_ADD, &sv);
}
