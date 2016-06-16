#ifndef _DRIVEWIRE_H
#define _DRIVEWIRE_H

struct dw_trans{
      	uint8_t *sbuf;      /* pointer to user-space send buffer */
	uint16_t sbufz;     /* size of send buffer */
	uint8_t *rbuf;      /* dittos for receive buffer */
	uint16_t rbufz;
};

/* ioctl */
#define DRIVEWIREC_TRANS 1  /* low-level transaction interface

#endif
