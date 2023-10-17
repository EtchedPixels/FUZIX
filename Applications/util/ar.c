#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <ar.h>


static void (*action)(void);
static unsigned verbose;
static struct ar_hdr hdr;
static int in_fd;
static unsigned code = 0;

static void check_header(void)
{
    char buf[8];
    if (read(in_fd, buf, SARMAG) != SARMAG ||
        memcmp(buf, ARMAG, SARMAG)) {
        fprintf(stderr, "ar: not a valid archive.\n");
        exit(1);
    }
}

static void seek_next(void)
{
    long len = atol(hdr.ar_size);
    if (len & 1)
        len++;
    if (lseek(in_fd, len, SEEK_CUR) < 0) {
        perror("seek");
        exit(1);
    }
}

static unsigned bad_hdr(void)
{
    if (hdr.ar_fmag[0] != 96 || hdr.ar_fmag[1] != '\n')
        return 1;
    return 0;
}

static int octal(const char *x)
{
    int n = 0;
    sscanf(x, "%o", &n);
    return n;
}

static char *modestr(int mode)
{
    static char buf[10];
    unsigned n = 0400;
    strcpy(buf, "---------");
    if (mode & 0400)
        buf[0] = 'r';
    if (mode & 0200)
        buf[1] = 'w';
    if (mode & 0100)
        buf[2] = 'x';
    if (mode & 0040)
        buf[3] = 'r';
    if (mode & 0020)
        buf[4] = 'w';
    if (mode & 0010)
        buf[5] = 'x';
    if (mode & 0004)
        buf[6] = 'r';
    if (mode & 0002)
        buf[7] = 'w';
    if (mode & 00001)
        buf[8] = 'x';
    /* To do stick and suid/gid */
    return buf;
}

static void list(void)
{
    if (verbose == 0)
        printf("%.16s\n", hdr.ar_name);
    else {
        char tbuf[32];
        time_t t = atol(hdr.ar_date);
        int uid = atoi(hdr.ar_uid);
        int gid = atoi(hdr.ar_gid);
        int mode = octal(hdr.ar_mode);
        unsigned long size = atol(hdr.ar_size);
        
        ctime_r(&t, tbuf);
        tbuf[24] = 0;
        printf("%9s %4d/%4d %6ld %s %.16s\n",
            modestr(mode),
            uid, gid, size, tbuf, hdr.ar_name);
    }
    seek_next();
}

static int process_ar(void)
{
    while(read(in_fd, &hdr, sizeof(hdr)) == sizeof(hdr)) {
        if (bad_hdr()) {
            fprintf(stderr,"ar: archive is corrupt.\n");
            exit(1);
        }
        action();
    }
}

static void usage(void)
{
    fprintf(stderr, "usage:...\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    char *p = argv[1];

    if (argc < 3)
        usage();
    
    switch(*p++) {
    case 't':
        action = list;
        break;
    default:
        usage();
    }
    while(*p) {
        switch(*p++) {
        case 'v':
            verbose = 1;
            break;
        default:
            usage();
        }
    }  
    in_fd = open(argv[2], O_RDONLY);
    if (in_fd == -1) {
        perror(argv[2]);
        exit(1);
    }
    check_header();
    process_ar();
    close(in_fd);
    return code;      
}