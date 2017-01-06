#ifndef __DEVRD_Z180_DOT_H__
#define __DEVRD_Z180_DOT_H__

/* minor device numbers */
#define RD_MINOR_RAM     0
#define RD_MINOR_ROM     1
#define RD_MINOR_PHYSMEM 2
#define NUM_DEV_RD       3

/* the assumption is that RAM follows ROM in the memory map */
#define DEV_RD_ROM_START (DEV_RD_ROM_SIZE-DEV_RD_ROM_PAGES)                 /* first page used by the ROM disk */
#define DEV_RD_RAM_START (DEV_RD_ROM_SIZE+DEV_RD_RAM_SIZE-DEV_RD_RAM_PAGES) /* first page used by the RAM disk */

/* public interface */
int rd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
int rd_open(uint8_t minor, uint16_t flags);

#ifdef DEVRD_PRIVATE

/* The basic operation supported by devrd_hw is a memory copy
 * Source is expressed as:
 *   rd_src_address   -- 32-bit absolute address in physical memory
 * Destination is expressed as:
 *   rd_dst_userspace -- boolean, false: process memory (userspace), true: kernel memory
 *   rd_dst_address   -- 16-bit address in target memory space
 * Size and direction expressed as:
 *   rd_cpy_count     -- number of bytes to copy
 *   rd_reverse       -- reverse direction of the copy operation, ie dst->src rather than src->dst
 */

extern uint32_t rd_src_address;
extern uint16_t rd_dst_address;
extern bool     rd_dst_userspace;
extern uint16_t rd_cpy_count;
extern uint8_t rd_reverse;      /* reverse the copy direction -- read - false, write - true */
void rd_page_copy(void);        /* in devrd_hw.s */
#endif

#endif /* __DEVRD_Z180_DOT_H__ */
