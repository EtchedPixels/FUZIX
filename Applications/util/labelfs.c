#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/super.h>

static union {
    struct fuzix_filesys fs;
    char buf[512];
} fs;


void usage(void)
{
    fprintf(stderr, "Usage: labelfs [-l label] [-g heads,cyls,secs[,blocksize[,skew]] path,\n");
    exit(1);
}

void set_fs_label(const char *p)
{
    strncmp(fs.fs.s_label_name, p, sizeof(fs.fs.s_label_name));
    fs.fs.s_props |= S_PROP_LABEL;
}

uint8_t makesecsize(uint16_t bsize)
{
    uint8_t n = 0;
    while(bsize > 128) {
        bsize >>= 1;
        n++;
    }
    if (bsize != 128) {
        fprintf(stderr, "Invalid block size.\n");
        exit(1);
    }
    return n;
}

void set_fs_geo(const char *p)
{
    unsigned int heads;
    unsigned int cyls;
    unsigned int sectors;
    unsigned int bsize;
    unsigned int skew;
    int n = sscanf(p, "%d,%d,%d,%d,%d", &heads,&cyls,&sectors,&bsize,&skew);
    if (n < 3)
        usage();
    if (bsize & (bsize - 1))
        usage();
    fs.fs.s_geo_heads = heads;
    fs.fs.s_geo_cylinders = cyls;
    fs.fs.s_geo_sectors = sectors;
    if (n > 3)
        fs.fs.s_geo_secsize = makesecsize(bsize);
    else
        fs.fs.s_geo_secsize = 2;
    if (n == 5)
        fs.fs.s_geo_skew = skew;
    else
        fs.fs.s_geo_skew = 0;
    fs.fs.s_props |= S_PROP_GEO;
}

int main(int argc, char *argv[])
{
    char *geo = NULL;
    char *label = NULL;
    int opt;
    int fd;

    while((opt = getopt(argc, argv, "l:g:")) != -1) {
        switch(opt) {
        case 'g':
            geo = optarg;
            break;
        case 'l':
            label = optarg;
            break;
        default:
            usage();
        }
    }
    if (optind != argc -1)
        usage();
    fd = open(argv[optind], O_RDWR, 0600);
    if (fd == -1) {
        perror(argv[optind]);
        exit(1);
    }
    if (lseek(fd, 512L , SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }
    if (read(fd, fs.buf, 512) != 512) {
        perror("read");
        exit(1);
    }
    if (fs.fs.s_fs.s_mounted != SMOUNTED) {
        fprintf(stderr, "%s: not a filesystem.\n", argv[optind]);
        exit(1);
    }
    /* Don't scribble on a mounted fs it won't end well in all cases */
    if (fs.fs.s_fs.s_fmod == FMOD_DIRTY) {
        fprintf(stderr, "%s: dirty filesystem.\n", argv[optind]);
        if (geo || label)
            exit(1);
    }
    if (!geo && !label) {
        if (fs.fs.s_props & S_PROP_LABEL)
            printf("Label: %.32s.\n", fs.fs.s_label_name);
        if (fs.fs.s_props & S_PROP_GEO)
            printf("Geometry: %d heads, %d cylinders\n",
                fs.fs.s_geo_heads, fs.fs.s_geo_cylinders);
            printf("          %d sectors, blocksize %$d skew %d.\n",
                fs.fs.s_geo_sectors, 128 << fs.fs.s_geo_secsize, fs.fs.s_geo_skew);
        return 0;
    }
    if (lseek(fd, 512L , SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }
    if (label)
        set_fs_label(label);
    if (geo)
        set_fs_geo(geo);
    if (write(fd, fs.buf, 512) != 512) {
        perror("write");
        exit(1);
    }
    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
    return 0;
}
