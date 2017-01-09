#ifndef __DEVMEM_DOT_H__
#define __DEVMEM_DOT_H__

/* /dev/mem -- access to CPU physical memory address space
 *
 * all arguments are in udata:
 * udata.u_offset -- physical address to read/write
 * udata.u_count  -- number of bytes to transfer
 * udata.u_base   -- address of buffer in user process memory
 */

unsigned int devmem_read(void);
unsigned int devmem_write(void);

#endif
