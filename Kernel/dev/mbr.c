#include <kernel.h>
#include <kdata.h>
#include <printf.h>

typedef struct {
    unsigned char status;
    unsigned char chs_first[3];
    unsigned char type;
    unsigned char chs_last[3];
    unsigned long lba_first;
    unsigned long lba_count;
} partition_table_entry_t;

#define MBR_ENTRY_COUNT 4
typedef struct {
    unsigned char bootcode[446];
    partition_table_entry_t partition[MBR_ENTRY_COUNT];
    unsigned int signature;
} master_boot_record_t;

void parse_partition_table(void *buffer, uint32_t *first_lba, uint8_t *slice_count, uint8_t max_slices)
{
    master_boot_record_t *mbr = (master_boot_record_t*)buffer;
    uint16_t slices = 0;
    uint8_t i;

    /* check for MBR table signature */
    if(mbr->signature != 0xaa55){
        kputs("no partition table\n");
        return;
    }

    /* look for a fuzix partition (type 0x5A) */
    for(i=0; i<MBR_ENTRY_COUNT; i++){
        if(mbr->partition[i].type == 0x5A){
            *first_lba = mbr->partition[i].lba_first;
            slices = mbr->partition[i].lba_count >> SLICE_SIZE_LOG2_SECTORS;
            if(slices > max_slices)
                slices = max_slices;
            *slice_count = slices;
            break;
        }
    }

    kprintf("%d slices\n", slices);
}
