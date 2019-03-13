#ifndef __DEVRD_DOT_H__
#define __DEVRD_DOT_H__

/* minor device numbers */
#define RD_MINOR_ROM     0
#define RD_MINOR_RAM     1
#define NUM_DEV_RD       2

/* public interface */
int rd_open(uint_fast8_t minor, uint16_t flags);
int rd_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
int rd_transfer(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);

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
extern uint8_t  rd_reverse;      /* reverse the copy direction: false=read, true=write */
void rd_platform_copy(void);     /* platform code provides this function */
#endif

#endif /* __DEVRD_DOT_H__ */
