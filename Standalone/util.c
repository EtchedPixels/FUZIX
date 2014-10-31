#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fuzix_fs.h"

int dev_fd;
int dev_offset;
struct u_data udata;

int fd_open(char *name)
{
    char *namecopy, *sd;
    int subdev = 0;

    namecopy=strdup(name);
    sd = index(namecopy, ':');
    if(sd){
        *sd = 0;
        sd++;
        subdev=atoi(sd);
    }

    printf("Opening %s sd%d\n", namecopy, subdev);
    dev_offset = subdev << 25; // * 32MB
    dev_fd = open(namecopy, O_RDWR | O_CREAT, 0666);
    free(namecopy);

    if(dev_fd < 0)
        return -1;
    printf("fd=%d, dev_offset = %d\n", dev_fd, dev_offset);
    return 0;
}


void panic(char *s)
{
    fprintf(stderr, "panic: %s\n", s);
    exit(1);
}
