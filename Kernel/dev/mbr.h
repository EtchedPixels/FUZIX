#ifndef __MBR_DOT_H__
#define __MBR_DOT_H__

/* 2015-01-04 Will Sowerbutts */

typedef struct __packed {
    /* Described this way so that it packs */
    uint8_t  status_chs_first[4];
    uint8_t  type_chs_last[4];
    uint32_t lba_first;
    uint32_t lba_count;
} partition_table_entry_t;

#define MBR_ENTRY_COUNT 4
#define MBR_SIGNATURE 0xAA55
typedef struct __packed {
    uint8_t bootcode[380];
    /* Last 66 bytes go to the command line */
    uint16_t cmdflag;
#define MBR_BOOT_CMD 0x5EB5
    uint8_t cmdline[64];
    partition_table_entry_t partition[MBR_ENTRY_COUNT];
    uint16_t signature;
} boot_record_t;

void mbr_parse(uint_fast8_t letter);

#define FUZIX_SWAP	0x7F

#endif /* __MBR_DOT_H__ */
