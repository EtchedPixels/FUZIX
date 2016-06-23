#ifndef _DRIVEWIRE_H
#define _DRIVEWIRE_H

/* Kernel definition */
uint16_t dw_transaction( uint8_t *send, uint16_t scnt,
			 uint8_t *recv, uint16_t rcnt, uint8_t rawf );

struct dw_trans{
      	uint8_t *sbuf;      /* pointer to user-space send buffer */
	uint16_t sbufz;     /* size of send buffer */
	uint8_t *rbuf;      /* dittos for receive buffer */
	uint16_t rbufz;
};

/* ioctl */
#define DRIVEWIREC_TRANS 0x4500  /* low-level transaction interface */

#endif
