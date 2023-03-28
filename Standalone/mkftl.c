#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include "../Kernel/lib/dhara/map.h"
#include "../Kernel/lib/dhara/nand.h"

static uint32_t pagesize = 512;
static uint32_t erasesize = 4096;
static uint32_t flashsize = 1024*1024; /* bytes */
static uint8_t gcratio = 2;

static uint8_t* flashdata;

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
                     dhara_error_t *err)
{
    memset(flashdata + (b*erasesize), 0xff, erasesize);
    *err = DHARA_E_NONE;
    return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data,
                    dhara_error_t *err)
{
    memcpy(flashdata + (p*pagesize), data, pagesize);
    *err = DHARA_E_NONE;
    return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
                    size_t offset, size_t length,
                    uint8_t *data,
                    dhara_error_t *err)
{
    if (p >= (flashsize/pagesize))
    {
        *err = DHARA_E_BAD_BLOCK;
        return -1;
    }

    memcpy(data, flashdata + (p*pagesize) + offset, length);
    *err = DHARA_E_NONE;
    return 0;
}

int dhara_nand_is_bad(const struct dhara_nand* n, dhara_block_t b)
{
    return 0;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b) {}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    const uint8_t* ptr = flashdata + (p*512);
    for (int i=0; i<512; i++)
        if (ptr[i] != 0xff)
            return 0;

    return 1;
}

int dhara_nand_copy(const struct dhara_nand *n,
                    dhara_page_t src, dhara_page_t dst,
                    dhara_error_t *err)
{
    const uint8_t* psrc = flashdata + (src*512);
    uint8_t* pdst = flashdata + (dst*512);
    memcpy(pdst, psrc, 512);
    *err = DHARA_E_NONE;
    return 0;
}

static void panic(const char* s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "Fatal: ");
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

static bool is_power_of_2(uint32_t i)
{
    return i && !((i-1) & i);
}

static int ilog2(uint32_t i)
{
    return ffsl(i) - 1;
}

static void syntax_error(void)
{
    fprintf(stderr, "Syntax: mkftl [options] <inputfile> -o <outputfile>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p <number>   set page size, in bytes (default: %d bytes)\n", pagesize);
    fprintf(stderr, "  -e <number>   set erase block size, in bytes (default: %d bytes)\n", erasesize);
    fprintf(stderr, "  -s <number>   set image size, in kilobytes (default: %d kB)\n", flashsize/1024);
    fprintf(stderr, "  -g <number>   set garbage collection ratio (default: %d)\n", gcratio);
    exit(1);
}

int main(int argc, char* const* argv)
{
    const char* outputfilename = NULL;
    const char* inputfilename = NULL;

    for (;;)
    {
        int opt = getopt(argc, argv, "o:p:e:s:g:");
        if (opt == -1)
            break;
        switch (opt)
        {
            case 'o':
                outputfilename = optarg;
                break;

            case 'p':
                pagesize = strtoul(optarg, NULL, 0);
                break;

            case 'e':
                erasesize = strtoul(optarg, NULL, 0);
                break;

            case 's':
                flashsize = strtoul(optarg, NULL, 0) * 1024;
                break;

            case 'g':
                gcratio = strtoul(optarg, NULL, 0);
                break;

            default:
                syntax_error();
        }
    }

    if (argc != (optind+1))
        syntax_error();
    inputfilename = argv[optind];

    if (!outputfilename)
        panic("output filename must be supplied");

    if (!is_power_of_2(pagesize))
        panic("page size %d is not a power of two", pagesize);

    if (!is_power_of_2(erasesize))
        panic("erase size %d is not a power of two", erasesize);

    if (erasesize % pagesize)
        panic("erase size is not a multiple of page size");

    if (flashsize % erasesize)
        panic("flash size is not a multiple of erase size");

    struct dhara_nand nand;
    nand.log2_page_size = ilog2(pagesize);
    nand.log2_ppb = ilog2(erasesize) - nand.log2_page_size;
    nand.num_blocks = flashsize / erasesize;

    flashdata = malloc(flashsize);
    memset(flashdata, 0xff, flashsize);

    uint8_t journal_buf[512];
    struct dhara_map dhara;
    dhara_map_init(&dhara, &nand, journal_buf, gcratio);
    dhara_error_t err = DHARA_E_NONE;
    dhara_map_resume(&dhara, &err);
    printf("Number of physical erase blocks: %d\n", nand.num_blocks);

    uint32_t lba = dhara_map_capacity(&dhara);
    printf("Maximum logical size: %dkB (%d sectors)\n", lba / 2, lba);

    FILE* inf = fopen(inputfilename, "rb");
    if (!inf)
        panic("cannot open input file: %s", strerror(errno));
    fseek(inf, 0, SEEK_END);
    int sectors = ftell(inf) / 512;
    fseek(inf, 0, SEEK_SET);
    if (sectors > lba)
    {
        fprintf(stderr, "Logical image too big (%d > %d)\n",
            sectors, lba);
        exit(1);
    }
    if (sectors == lba)
    {
        fprintf(stderr, "WARNING: logical image completely fills the FTL filesystem; you\n");
        fprintf(stderr, "will have GC thrash if you try to write to it");
    }

    for (int sectorno=0; sectorno<sectors; sectorno++)
    {
        uint8_t buffer[512] = {0};
        if (fread(buffer, 512, 1, inf) != 1)
        {
            perror("FTL read error");
            exit(1);
        }

        err = DHARA_E_NONE;
        dhara_map_write(&dhara, sectorno, buffer, &err); 

        if (err != DHARA_E_NONE)
        {
            fprintf(stderr, "FTL error %d\n", err);
            exit(1);
        }
    }
    fclose(inf);

    err = DHARA_E_NONE;
    dhara_map_gc(&dhara, &err);
    uint32_t used = dhara_map_size(&dhara);
    printf("Used space: %dkB (%d sectors)\n", used / 2, used);

    FILE* outf = fopen(outputfilename, "wb");
    if (!outf)
        panic("cannot open output file: %s", strerror(errno));
    if (fwrite(flashdata, 1, flashsize, outf) == flashsize && fclose(outf) == 0)
        return 0;
    panic("Short write on %s.\n", outputfilename);
}

// vim: sw=4 ts=4 et:

