#ifndef __MBR_DOT_H__
#define __MBR_DOT_H__

#include <sys/compiler.h>

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
    uint8_t bootcode[446];
    partition_table_entry_t partition[MBR_ENTRY_COUNT];
    uint16_t signature;
} boot_record_t;

#endif /* __MBR_DOT_H__ */
