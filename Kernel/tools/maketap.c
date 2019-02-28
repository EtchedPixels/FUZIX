/*
 *	Turn some code into a .tap file
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static void dowrite(int fd, const uint8_t *buf, int len, const char *name)
{
    int l = write(fd, buf, len);
    if (l == len)
        return;
    if (l < 0) {
        perror(name);
        exit(1);
    }
    fprintf(stderr, "%s: short write.\n", name);
    exit(1);
}

static void writecsum(int fd, const uint8_t *block, int len, const char *name)
{
    uint8_t csum = 0;
    const uint8_t *p = block;
    int ct = 0;

    while(ct++ < len)
        csum ^= *p++;
    dowrite(fd, &csum, 1, name);
}

static uint16_t getu16(const char *prog, const char *p)
{
    int n;
    if (sscanf(p, "%d", &n) == 0 || n < 0 || n > 65535) {
        fprintf(stderr, "%s: '%s' is not a valid value.\n", prog, p);
        exit(1);
    }
    return (uint16_t)n;
}

int main(int argc, char *argv[])
{
    uint16_t address;
    uint16_t len;
    char *name;
    int ifd, ofd;
    uint8_t header[17];
    uint8_t buf[65536];
    int ct = 1;

    if (argc != 5) {
        fprintf(stderr, "%s: input output address length.\n", argv[0]);
        exit(1);
    }
    address = getu16(argv[0], argv[3]);
    len = getu16(argv[0], argv[4]);
    if (address + len < address|| len + 2 < len ) {
        fprintf(stderr, "%s: size would wrap.\n", argv[0]);
        exit(1);
    }

    if ((name = strrchr(argv[2], '/')) == NULL)
        name = argv[2];
    else
        name++;

    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "%s: output must differ from input.\n", argv[0]);
        exit(1);
    }

    ifd = open(argv[1], O_RDONLY);
    if (ifd == -1) {
        perror(argv[1]);
        exit(1);
    }
    
    if (read(ifd, buf, len) != len) {
        fprintf(stderr, "%s: unable to read %d bytes from %s.\n",
            argv[0], len, argv[1]);
    }

    ofd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (ofd == -1) {
        perror(argv[2]);
        exit(1);
    }

    header[0] = 19;	/* 19 bytes for the header (not including this length) */
    header[1] = 0;	/* Length high */
    header[2] = 0;	/* And it's a header block */
    dowrite(ofd, header, 3, argv[2]);
    
    /* Now write out the ZX Spectrum header data */
    
    header[0] = 3;	/* CODE */
    while(*name && ct <= 10)
        header[ct++] = *name++;
    while(ct <= 10)
        header[ct++] = ' ';
    header[11] = len;
    header[12] = len >> 8;
    header[13] = address;
    header[14] = address >> 8;
    header[15] = 0x00;		/* As the machine writes them */
    header[16] = 0x80;
    dowrite(ofd, header, 17, argv[2]);
    writecsum(ofd, header, 17, argv[2]);    

    /* The second block is then the actual code */
    header[0] = (len + 2);
    header[1] = (len + 2) >> 8;
    header[2] = 0xff;
    dowrite(ofd, header, 3, argv[2]);
    /* Now the code */
    dowrite(ofd, buf, len, argv[2]);
    /* Checksum - add in the 0xFF for the flag */
    buf[len] = 0xFF;
    writecsum(ofd, buf, len+1, argv[2]);
    close(ofd);
    close(ifd);
    return 0;
}
