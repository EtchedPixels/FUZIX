/*
 *	COCO virtual images are stored one 256 byte sector per 512 byte IDE sector. Explode the
 *	disk image input accordingly
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void write256(const char *buf)
{
    if (write(1, buf, 256) != 256) {
        fprintf(stderr, "diskpad: write error.\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    char buf[256];
    int l;
    while((l = read(0, buf, 256)) == 256) {
        write256(buf);
        write256(buf);
    }
    if (l == -1)
        perror("read");
    else if (l)
        fprintf(stderr, "diskpad: unexpected trailing bytes.\n");
    return !!l;
}

        
        