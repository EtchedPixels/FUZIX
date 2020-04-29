/* 
    FLASHROM: in-system flash ROM programmer for FUZIX
    (c) Will Sowerbutts <will@sowerbutts.com> 2017-01-02
    Based on my FLASH4 and FLASH030 utilities
    GPL 2 License
*/

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FLASHROM_PHYSICAL_BASE   0  /* Location in the physical address space */
#define SLOW_BUT_SAFE            0  /* Set to 1 for very fast systems */

typedef enum { 
    ACTION_UNKNOWN, 
    ACTION_READ, 
    ACTION_WRITE, 
    ACTION_VERIFY 
} action_t;

static action_t action = ACTION_UNKNOWN;
bool allow_partial=false;
int mem_fd = -1;
int img_fd = -1;
unsigned long file_size;

typedef struct {
    unsigned int chip_id;
    char *chip_name;
    unsigned long sector_size;  /* bytes */
    unsigned int sector_count;
    unsigned char strategy;
} flashrom_chip_t; 

/* the strategy flags describe quirks for programming particular chips */
#define ST_NORMAL               (0x00) /* default: no special strategy required */
#define ST_PROGRAM_SECTORS      (0x01) /* bit 0: program sector (not byte) at a time (Atmel AT29C style) */
#define ST_ERASE_CHIP           (0x02) /* bit 1: erase whole chip (sector_count must be exactly 1) instead of individual sectors */

static flashrom_chip_t flashrom_chips[] = {
    { 0x0120, "29F010",        16384,    8, ST_NORMAL },
    { 0x01A4, "29F040",        65536,    8, ST_NORMAL },
    { 0x1F04, "AT49F001NT",   131072,    1, ST_ERASE_CHIP }, /* multiple but unequal sized sectors */
    { 0x1F05, "AT49F001N",    131072,    1, ST_ERASE_CHIP }, /* multiple but unequal sized sectors */
    { 0x1F07, "AT49F002N",    262144,    1, ST_ERASE_CHIP }, /* multiple but unequal sized sectors */
    { 0x1F08, "AT49F002NT",   262144,    1, ST_ERASE_CHIP }, /* multiple but unequal sized sectors */
    { 0x1F13, "AT49F040",     524288,    1, ST_ERASE_CHIP }, /* single sector device */
    { 0x1F5D, "AT29C512",        128,  512, ST_PROGRAM_SECTORS },
    { 0x1FA4, "AT29C040",        256, 2048, ST_PROGRAM_SECTORS },
    { 0x1FD5, "AT29C010",        128, 1024, ST_PROGRAM_SECTORS },
    { 0x1FDA, "AT29C020",        256, 1024, ST_PROGRAM_SECTORS },
    { 0x2020, "M29F010",       16384,    8, ST_NORMAL },
    { 0x20E2, "M29F040",       65536,    8, ST_NORMAL },
    { 0xBFB5, "39F010",         4096,   32, ST_NORMAL },
    { 0xBFB6, "39F020",         4096,   64, ST_NORMAL },
    { 0xBFB7, "39F040",         4096,  128, ST_NORMAL },     /* the author uses and recommends this device */
    { 0xC2A4, "MX29F040",      65536,    8, ST_NORMAL },
    /* terminate the list */
    { 0x0000, NULL,            0,    0, 0 }
};

static flashrom_chip_t *flashrom_type = NULL;
static unsigned long flashrom_size; /* bytes */

/* DATA_BUFFER_SIZE:
 *  - must be a power of 2
 *  - must be at least as large as the largest sector for any chip using ST_PROGRAM_SECTORS
 *  - 512 bytes or more allows Fuzix to load data direct into userspace saving one copy */
#define DATA_BUFFER_SIZE 512
unsigned char data_buffer[DATA_BUFFER_SIZE];
unsigned char rom_buffer[DATA_BUFFER_SIZE];

/* useful to provide some feedback that something is actually happening with large-sector devices */
#define SPINNER_LENGTH 4
static char spinner_char[SPINNER_LENGTH] = {'|', '/', '-', '\\'};
static unsigned char spinner_pos=0;

char spinner(void)
{
    spinner_pos = (spinner_pos + 1) % SPINNER_LENGTH;
    return spinner_char[spinner_pos];
}

unsigned char fr_chip_read(unsigned long address)
{
    unsigned char buf;
    lseek(mem_fd, FLASHROM_PHYSICAL_BASE + address, SEEK_SET);
    if(read(mem_fd, &buf, 1) != 1)
        printf("read from /dev/mem failed: %s\n", strerror(errno));
    return buf;
}

void fr_chip_read_block(unsigned long address, void *buffer, int length)
{
    lseek(mem_fd, FLASHROM_PHYSICAL_BASE + address, SEEK_SET);
    if(read(mem_fd, buffer, length) != length)
        printf("read from /dev/mem failed: %s\n", strerror(errno));
}

void fr_chip_write(unsigned long address, unsigned char value)
{
    lseek(mem_fd, FLASHROM_PHYSICAL_BASE + address, SEEK_SET);
    if(write(mem_fd, &value, 1) != 1)
        printf("write to /dev/mem failed: %s\n", strerror(errno));
}

void abort_and_solicit_report(void)
{
    puts("Please email will@sowerbutts.com if you would like support for your\nsystem added to this program.\n");
    _exit(1);
}

unsigned long fr_sector_address(unsigned int sector)
{
    return flashrom_type->sector_size * ((unsigned long)sector);
}

void fr_wait_toggle_bit(unsigned long address)
{
    unsigned char a, b;

    /* wait for toggle bit to indicate completion */
    do{
        a = fr_chip_read(address);
        b = fr_chip_read(address);
        if(a==b){
            /* data sheet says two additional reads are required */
            a = fr_chip_read(address);
            b = fr_chip_read(address);
        }
    }while(a != b);
}

void fr_chip_write_block(unsigned long address, unsigned int length)
{
    unsigned char *buffer = data_buffer;
    unsigned char magic1 = 0xAA;
    unsigned char magic2 = 0x55;
    unsigned char magic3 = 0xA0;
#if SLOW_BUT_SAFE
    unsigned char test;
#endif

    while(length--){
        if(*buffer != 0xFF){
            // * enter programming mode
            // flashrom_chip_write(0x5555, 0xAA);
            lseek(mem_fd, FLASHROM_PHYSICAL_BASE + 0x5555, SEEK_SET);
            write(mem_fd, &magic1, 1);
            // flashrom_chip_write(0x2AAA, 0x55);
            lseek(mem_fd, FLASHROM_PHYSICAL_BASE + 0x2AAA, SEEK_SET);
            write(mem_fd, &magic2, 1);
            // flashrom_chip_write(0x5555, 0xA0);
            lseek(mem_fd, FLASHROM_PHYSICAL_BASE + 0x5555, SEEK_SET);
            write(mem_fd, &magic3, 1);

            // * write the byte
            // flashrom_chip_write(address, *buffer);
            lseek(mem_fd, FLASHROM_PHYSICAL_BASE + address, SEEK_SET);
            write(mem_fd, buffer, 1);

            // * wait for programming to complete 
            // the data sheet advises you check this twice but this boat is so
            // slow there's not much point doing it at all!
#if SLOW_BUT_SAFE
            while(fr_chip_read(address) != *buffer);
            while(fr_chip_read(address) != *buffer);
            do{
                lseek(mem_fd, FLASHROM_PHYSICAL_BASE + address, SEEK_SET);
                read(mem_fd, &test, 1);
            }while(test != *buffer);
#endif
        }
        buffer++;
        address++;
    }
}

void fr_chip_erase(void)
{
    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(0x5555, 0x80);
    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(0x5555, 0x10);
    fr_wait_toggle_bit(0);
}

void fr_sector_erase(unsigned long address)
{
    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(0x5555, 0x80);
    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(address, 0x30);
    fr_wait_toggle_bit(address);
}

/* this is used only for programming atmel 29C parts which have a combined erase/program cycle */
void fr_sector_program(unsigned long address, unsigned int count)
{
    unsigned long prog_address;
    unsigned char *buffer = data_buffer;

    prog_address = address;

    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(0x5555, 0xA0); /* software data protection activated */

    while(count--){
        fr_chip_write(prog_address++, *(buffer++));
    }

    fr_wait_toggle_bit(address);
}

bool fr_identify(void)
{
    unsigned int flashrom_device_id;

    /* put the flash memory into identify mode */
    fr_chip_write(0x5555, 0xAA);
    fr_chip_write(0x2AAA, 0x55);
    fr_chip_write(0x5555, 0x90);

    /* 2017-01-03 removed 2 x _pause(1) -- does this fix the intermittent failure to identify the ROM? */

    /* atmel 29C parts require a pause for 10msec at this point */
    // _pause(1); /* shortest delay we can request is 0.1 seconds */

    /* load manufacturer and device IDs */
    flashrom_device_id = (((unsigned int)fr_chip_read(0x0000) & 0xFF) << 8) 
                         | (fr_chip_read(0x0001) & 0xFF);

    /* put the flash memory back into normal mode */
    fr_chip_write(0x5555, 0xF0);

    /* atmel 29C parts require a pause for 10msec at this point */
    // _pause(1); /* shortest delay we can request is 0.1 seconds */

    printf("Flash memory chip ID is 0x%04X: ", flashrom_device_id);

    for(flashrom_type = flashrom_chips; flashrom_type->chip_id; flashrom_type++)
        if(flashrom_type->chip_id == flashrom_device_id)
            break;

    if(!flashrom_type->chip_id){
        /* we scanned the whole table without finding our chip */
        flashrom_type = NULL;
        puts("Unknown flash chip.");
        return false;
    }else{
        printf("%s\n", flashrom_type->chip_name);
        return true;
    }
}

void fr_read(void)
{
    unsigned long offset;
    ssize_t w;

    lseek(img_fd, 0, SEEK_SET);

    offset = 0;
    while(offset < flashrom_size){
        printf("\rRead %d/%dKB ", (int)(offset >> 10), (int)(flashrom_size >> 10));
        fflush(stdout);
        fr_chip_read_block(offset, data_buffer, DATA_BUFFER_SIZE);
        w = write(img_fd, data_buffer, DATA_BUFFER_SIZE);
        if(w != DATA_BUFFER_SIZE){
            printf("write() failed: %s\n", strerror(errno));
            _exit(1);
        }
        offset += DATA_BUFFER_SIZE;
    }

    puts("\rRead complete.   ");
}

unsigned long loaded_address = -1;
void load_block(unsigned long address, unsigned int block_size)
{
    int r;

    if(loaded_address == address)
        return;

    lseek(img_fd, address, SEEK_SET);
    r = read(img_fd, data_buffer, block_size);
    if(r < 0){
        printf("read() failed: %s\n", strerror(errno));
        exit(1);
    }

    /* pad if necessary */
    if(r < block_size){
        memset(&data_buffer[r], 0xFF, block_size-r);
    }

    loaded_address = address;
}

bool fr_verify_and_write(bool perform_write)
{
    bool sector_match;
    unsigned int mismatch=0, sector, block, block_size, blocks_per_sector;
    unsigned long rom_address;

    if(DATA_BUFFER_SIZE > flashrom_type->sector_size){
        block_size = flashrom_type->sector_size;
        blocks_per_sector = 1;
    }else{
        block_size = DATA_BUFFER_SIZE;
        blocks_per_sector = flashrom_type->sector_size / DATA_BUFFER_SIZE;
    }

    for(sector=0; sector<flashrom_type->sector_count; sector++){
        printf("\r%s: sector %d/%d   ", perform_write ? "Write" : "Verify", sector, flashrom_type->sector_count);
        fflush(stdout);
        rom_address = fr_sector_address(sector);

        /* check for EOF */
        if(rom_address >= file_size)
            break;

        /* verify sector */
        sector_match = true;
        for(block=0; sector_match && block<blocks_per_sector; block++){
            load_block(rom_address, block_size);
            fr_chip_read_block(rom_address, rom_buffer, block_size);
            if(memcmp(data_buffer, rom_buffer, block_size)){
                sector_match = false;
            }
            rom_address += block_size;
        }

        if(!sector_match){
            mismatch++;
            rom_address = fr_sector_address(sector); /* rewind to start of sector */
            if(perform_write){
                /* erase and program sector */
                if(flashrom_type->strategy & ST_PROGRAM_SECTORS){
                    /* This type of chip has a combined erase/program cycle that programs a whole
                       sector at once. The sectors are quite small (128 or 256 bytes) so there is
                       exactly 1 subsector. */
                    fr_sector_program(rom_address, block_size);
                }else{
                    if(flashrom_type->strategy & ST_ERASE_CHIP)
                        fr_chip_erase(); /* only 1 sector with this chip type */
                    else
                        fr_sector_erase(rom_address);

                    for(block=0; block<blocks_per_sector; block++){
                        putchar(0x08);
                        putchar(spinner());
                        fflush(stdout);
                        load_block(rom_address, block_size);
                        fr_chip_write_block(rom_address, block_size);
                        rom_address += block_size;
                    }
                }
            }
        }
    }

    /* report outcome */
    if(perform_write){
        printf("\rWrite complete: Reprogrammed %d/%d sectors.\n", mismatch, flashrom_type->sector_count);
    }else{
        if(sector != flashrom_type->sector_count)
            printf("\rPartial verify (%d/%d sectors)", sector, flashrom_type->sector_count);
        else
            printf("\rVerify (%d sectors)", flashrom_type->sector_count);

        if(mismatch){
            printf(" complete: %d sectors contain errors.\n", mismatch);
            printf("\n*** VERIFY FAILED ***\n\n");
        }else
            printf(" complete: OK!\n");
    }

    return (mismatch > 0);
}

bool check_file_size(void)
{
    file_size = lseek(img_fd, 0, SEEK_END);
    lseek(img_fd, 0, SEEK_SET);

    if(file_size == flashrom_size)
        return true;

    if(file_size > flashrom_size){
        puts("ROM image file is larger than flash ROM");
        return false;
    }

    if(allow_partial &&
       (flashrom_size > file_size) && 
       (file_size != 0))
        return true;

    puts("ROM image file is smaller than flash ROM\n" \
         "You may use '--partial' to program only part of the ROM\n" \
         "from this file.");

    return false;
}

bool map_flashrom(void)
{
    mem_fd = open("/dev/mem", O_RDWR);

    if(mem_fd < 0){
        printf("Cannot open /dev/mem: %s\n", strerror(errno));
        return false;
    }

    return true;
}

void unmap_flashrom(void)
{
    close(mem_fd);
    mem_fd = -1;
}

void usage(const char *cmdname)
{
    printf("Usage: %s [OPTION...] COMMAND filename\n\n" \
           "OPTION:\n" \
           " -h --help      This usage summary\n" \
           " -p --partial   Allow ROM and file sizes to differ\n\n" \
           "COMMAND:\n" \
           " -r --read      Read ROM conents out to file\n" \
           " -v --verify    Compare ROM contents to file\n" \
           " -w --write     Rewrite ROM contents from file\n", cmdname);
}

int main(int argc, const char *argv[])
{
    int i;
    bool mismatch;
    const char *filename = NULL;
    action_t new_action;
    puts("flashrom by Will Sowerbutts <will@sowerbutts.com> version 1.0.0");

    sync(); // just in case we break something

    // command line arguments
    for(i=1; i<argc; i++){
        new_action = ACTION_UNKNOWN;
        if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--partial") == 0){
            allow_partial = true;
        }else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
            usage(argv[0]);
            return 0;
        }else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--read") == 0){
            new_action = ACTION_READ;
        }else if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--write") == 0){
            new_action = ACTION_WRITE;
        }else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verify") == 0){
            new_action = ACTION_VERIFY;
        }else{
            if(filename == NULL)
                filename = argv[i];
            else{
                printf("Unrecognised option \"%s\"\n", argv[i]);
                usage(argv[0]);
                return 1;
            }
        }

        if(new_action != ACTION_UNKNOWN){
            if(action == ACTION_UNKNOWN){
                action = new_action;
            }else{
                puts("More than one command specified!");
                usage(argv[0]);
                return 1;
            }
        }
    }
    
    if(action == ACTION_UNKNOWN){
        puts("No command specified!");
        usage(argv[0]);
        return 1;
    }

    if(!map_flashrom())
        return 1;

    /* identify flash ROM chip */
    if(!fr_identify()){
        puts("Your flash memory chip is not recognised.");
        abort_and_solicit_report();
    }

    flashrom_size = flashrom_type->sector_size * (unsigned long)flashrom_type->sector_count;

    printf("Flash memory has %d sectors of %ld bytes, total %dKB\n", 
            flashrom_type->sector_count, flashrom_type->sector_size,
            flashrom_size >> 10);

    /* execute action */
    switch(action){
        case ACTION_READ:
            img_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(img_fd < 0){
                printf("Cannot create image file \"%s\": %s\n", filename, strerror(errno));
                return 1;
            }
            fr_read();
            break;
        case ACTION_VERIFY:
        case ACTION_WRITE:
            img_fd = open(filename, O_RDONLY);
            if(img_fd < 0){
                printf("Cannot open image file \"%s\": %s\n", filename, strerror(errno));
                return 1;
            }
            if(!check_file_size())
                return 1;
            if(action == ACTION_VERIFY)
                mismatch = true; /* force verify */
            else /* ACTION_WRITE */
                mismatch = fr_verify_and_write(true); /* we avoid verifying if nothing changed */
            if(mismatch)
                fr_verify_and_write(false);
            break;
        default:
            puts("bug: unknown action");
            _exit(1);
    }

    unmap_flashrom();

    if(img_fd >= 0)
        close(img_fd);

    return 0;
}

