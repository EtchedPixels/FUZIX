#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t disk[655360];
static uint8_t blockbuf[65537];
static unsigned int disk_len;

struct catalogue {
    uint8_t name[8];
    uint8_t type;
    uint16_t addr;		/* LE */
    uint16_t len;		/* LE */
    uint8_t seclen;		/* LE */
    uint8_t sector;
    uint8_t track;
} __attribute((packed));

static void scan_catalogue(void)
{
    struct catalogue *c = (struct catalogue *)disk;
    int n = 128;
    printf("Volume: %8.8s\n", disk + 8 * 256 + 245);

    while(n-- && c->name[0]) {
        printf("Addr %5d Length %5d Sectors %3d From %2d/%2d ",
            c->addr, c->len, c->seclen, c->sector, c->track);
        if (c->name[0] == 1)
            printf("(Del)\n");
        else
            printf("%8.8s.%c\n",
                c->name, c->type);
        c++;
    }
}
    
static int namecomp(const char *name, const uint8_t *cname)
{
    int n = 0;
    while (n++ < 8) {
        if (*name == 0) {
            while(n++ < 8) {
                if (*cname++ != ' ')
                    return 0;
            }
            return 1;
        }
        if (*name++ != *cname++)
            return 0;
    }
    return 1;
}            
    
static struct catalogue *find_file(const char *name)
{
    struct catalogue *c = (struct catalogue *)disk;
    int n = 128;
    while(n-- && c->name[0]) {
        if (namecomp(name, c->name))
            return c;
        c++;
    }
    return NULL;
}
    
    
static void load_base_trd(const char *name)
{
    int fd = open(name, O_RDONLY);
    int len;

    if (fd == -1) {
        perror(name);
        exit(1);
    }

    len = read(fd, disk, sizeof(disk));
    if (len < 9 * 256) {
        fprintf(stderr, "%s: too short to be valid.\n", name);
        exit(1);
    }
    close(fd);
    
    disk_len = len;
    
    if (disk[8 * 256 + 231] != 0x10) {
        fprintf(stderr, "%s: not a TRDOS disk.\n", name);
        exit(1);
    }

    if (disk[8 * 256 + 227] != 0x16) {
        fprintf(stderr, "%s: only DSDD images supported.\n", name);
        exit(1);
    }

    scan_catalogue();
}

static void write_new_trd(const char *name)
{
    int fd = open(name, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    int len;

    if (fd == -1) {
        perror(name);
        exit(1);
    }

    len = write(fd, disk, sizeof(disk));
    if (close(fd) || len != sizeof(disk)) {
        fprintf(stderr, "%s: short write.\n", name);
        exit(1);
    }
}

static unsigned int load_block(const char *name)
{
    int fd = open(name, O_RDONLY);
    int len;
    if (fd == -1) {
        perror(name);
        exit(1);
    }

    len = read(fd, blockbuf, sizeof(blockbuf));
    close(fd);

    if (len == -1) {
        perror(name);
        exit(1);
    }
    if (len == 65537) {
        fprintf(stderr, "%s: block too large.\n", name);
        exit(1);
    }
    if (len == 0) {
        fprintf(stderr, "%s: empty.\n", name);
        exit(1);
    }
    return len;
}

/* Replace the data in the given pre-prepared block with ours. Easy as the disk
   is in media order and files are kept linearly */
static void insert_data(const char *name, uint8_t type, uint16_t len)
{
    unsigned int lsec;
    struct catalogue *c = find_file(name);
    if (c == NULL) {
        fprintf(stderr, "%s: not found.\n", name);
        exit(1);
    }
    if (c->type != type) {
        fprintf(stderr, "%s: wrong type (%c).\n", name, c->type);
        exit(1);
    }
    lsec = ((uint16_t)c->track) << 4;
    lsec |= c->sector;
    if (len != c->len) {
        fprintf(stderr, "%s: should be %d bytes not %d.\n", name, c->len, len);
        exit(1);
    }
    memcpy(disk + lsec * 256, blockbuf, len);
}

int main(int argc, char *argv[])
{
    unsigned int len;
    if (argc != 4) {
        fprintf(stderr, "%s image name data.\n", argv[0]);
        exit(1);
    }
    load_base_trd(argv[1]);
    len = load_block(argv[3]);
    insert_data(argv[2], 'C', len);
    write_new_trd(argv[1]);
    return 0;
}
