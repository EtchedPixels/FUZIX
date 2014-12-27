#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/*
 * 2014-12-21 William R Sowerbutts
 *
 * Create a CP/M program which boots Fuzix.
 *
 * The CP/M executable file contains a small loader program, followed by some
 * data describing where the kernel should be loaded, and then the kernel
 * itself.
 *
 * When run, the CP/M program copies a small (20 byte) program to the very
 * start of memory, which then copies the kernel to the correct address and
 * executes it.
 *
 *
 * Syntax: makecpmloader <loader> <kernel> <address> <output>
 *
 * loader   -- normally cpmload.bin, assembled from cpmload.s
 * kernel   -- fuzix.bin from the Fuzix build process
 * address  -- the kernel's load address (normally 0x88)
 * output   -- output file name
 *
 */

#define LOADER_TRIM       0x100      /* CP/M loads us at 0x100 */
#define MAX_LOADER_LENGTH 0x200      /* Loader should really be teeny tiny */
#define MAX_KERNEL_LENGTH 0x10000    /* Kernel can't be larger than 64KB ... yet ... */
#define BYTESWAP16(x)  (((x>>8) & 0xFF) | ((x & 0xFF) << 8))

char loader_data[MAX_LOADER_LENGTH];
char kernel_data[MAX_KERNEL_LENGTH];

struct loader_trailer {
    unsigned short load_address;
    unsigned short load_length;
};

int load_file(const char *filename, char *buffer, int buffer_len)
{
    int fd, length, r;

    fd = open(filename, O_RDONLY);

    if(fd < 0){
        fprintf(stderr, "Cannot open \"%s\": %s\n", filename, strerror(errno));
        return -1;
    }

    length = 0;
    while(1){
        r = read(fd, &buffer[length], buffer_len - length);
        if(r == 0) /* EOF */
            break;
        else if(r > 0)
            length += r;
        else{
            fprintf(stderr, "Cannot read \"%s\": %s\n", filename, strerror(errno));
            close(fd);
            return -1;
        }
        if(length == buffer_len){
            fprintf(stderr, "Out of buffer space reading \"%s\"\n", filename);
            close(fd);
            return -1;
        }
    }

    close(fd);
    return length;
}

int parse_int(char *str)
{
    int base = 10;
    long val;
    char *end;

    end = str;
    if(strncasecmp(end, "0x", 2) == 0){
        base = 16;
        end = end + 2;
    }

    val = strtol(end, &end, base);

    if(*end == 0 && *str != 0)
        return val;
    else{
        fprintf(stderr, "Cannot parse load address \"%s\"\n", str);
        return -1;
    }
}

int main(int argc, char **argv)
{
    int loader_length;
    int load_address;
    int kernel_length;
    int fd;
    struct loader_trailer trailer;

    if(argc < 5){
        fprintf(stderr, "%s [loader] [kernel] [address] [output]\n", argv[0]);
        return 1;
    }

    loader_length = load_file(argv[1], loader_data, MAX_LOADER_LENGTH);
    if(loader_length < 0)
        return 1;
    if(loader_length <= LOADER_TRIM){
        fprintf(stderr, "Loader image is too small\n");
        return 1;
    }

    kernel_length = load_file(argv[2], kernel_data, MAX_KERNEL_LENGTH);
    if(kernel_length < 0)
        return 1;

    load_address = parse_int(argv[3]);

    if(load_address < 0 || load_address > 0xFFFF){
        fprintf(stderr, "Bad load address.\n");
        return 1;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    trailer.load_length = kernel_length;
    trailer.load_address = load_address;
#else
    trailer.load_length = BYTESWAP16(kernel_length);
    trailer.load_address = BYTESWAP16(load_address);
#endif

    fd = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd < 0){
        fprintf(stderr, "Cannot open \"%s\": %s\n", argv[4], strerror(errno));
        return 1;
    }

    if(write(fd, &loader_data[LOADER_TRIM], loader_length - LOADER_TRIM) != (loader_length - LOADER_TRIM) ||
       write(fd, &trailer, sizeof(trailer)) != sizeof(trailer) ||
       write(fd, kernel_data, kernel_length) != kernel_length){
        fprintf(stderr, "Write to \"%s\" failed: %s\n", argv[4], strerror(errno));
        close(fd);
        unlink(argv[4]);
        return 1;
    }

    close(fd);

    return 0;
}
