#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


int do_mknod(char *path, char *modes, char *devs)
{
    int mode;
    int dev;

    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
        printf("mknod: bad mode\n");
        return (-1);
    }
    
    if (!S_ISFIFO(mode) && !S_ISDEV(mode)) {
        printf("mknod: mode is not device/fifo\n");
        return (-1);
    }

    if (sscanf(devs, "%d", &dev) != 1) {
        printf("mknod: bad device\n");
        return (-1);
    }

    if (mknod(path, mode, dev) != 0) {
        perror("mknod");
        return (-1);
    }
    return(0);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage: mknod path modes devs\n");
        return 1;
    }
    return do_mknod(argv[1], argv[2], argv[3]);
}
