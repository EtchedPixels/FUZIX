#ifndef __MBR_DOT_H__
#define __MBR_DOT_H__

void parse_partition_table(void *buffer, uint32_t *first_lba, uint8_t *slice_count, uint8_t max_slices);

#endif
