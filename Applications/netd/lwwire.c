/* 

   Network via the LWWire standard for DW4

*/
  
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/drivewire.h>

#include "device.h"

static int fd;  /* fd of drivewire block device fd */

/* returns number of byte in next packet, 0 = nothing waiting */
static int dwnet_poll( void )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[3];
    buf[0] = 0xf3;   /* lwwire's extension op */
    buf[1] = 0x01;   /* extension no */
    buf[2] = 0x00;   /* op = poll */
    d.sbuf = buf;
    d.sbufz = 3;
    d.rbuf = buf;
    d.rbufz = 2;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 ){
	return -1;
    }
    ret = (buf[0]<<8) + buf[1];
    if (ret<0) return 0;
    return ret;
}


/* drop the current packet */
static int dwnet_drop( void )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[3];
    buf[0] = 0xf3;   /* lwwire's extension op */
    buf[1] = 0x01;   /* extension no */
    buf[2] = 0x03;   /* op = drop */
    d.sbuf = buf;
    d.sbufz = 3;
    d.rbuf = NULL;
    d.rbufz = 0;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 ){
	return -1;
    }
    return 0;
}

/* receive packet to a buffer */
static int dwnet_recv( unsigned char *b, int len )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[3];
    buf[0] = 0xf3;   /* lwwire's extension op */
    buf[1] = 0x01;   /* extension no */
    buf[2] = 0x01;   /* op = read */
    d.sbuf = buf;
    d.sbufz = 3;
    d.rbuf = b;
    d.rbufz = len;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 ){
	return -1;
    }
    return 0;
}

/* received a chunked packets */
static int dwnet_recv_chunk( unsigned char *b, int size )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[3];
    buf[0] = 0xf3;   /* lwwire's extension op */
    buf[1] = 0x01;   /* extension no */
    buf[2] = 0x07;   /* op = read chunk */
    d.sbuf = buf;
    d.sbufz = 3;
    d.rbuf = b;
    d.rbufz = size;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 ){
	return -1;
    }
    return 0;
}

/* send packet to net device */
int device_send( char *sbuf, int len )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[5];
    buf[0] = 0xf3;   /* lwwire's extension op */
    buf[1] = 0x01;   /* extension no */
    buf[2] = 0x02;   /* op = send */
    buf[3] = len >> 8;
    buf[4] = len & 0xff ;
    d.sbuf = buf;
    d.sbufz = 5;
    d.rbuf = 0;
    d.rbufz = 0;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 )
	return -1;
    d.sbuf = sbuf;
    d.sbufz = len;
    d.rbuf = 0;
    d.rbufz = 0;
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if( ret < 0 )
	return -1;
    return 0;
}

#define CHUNKZ 511


int device_read( char *buf, int len )
{
    int packetz;
    int l;
    int s;
    l = packetz = dwnet_poll();
    if( l > len ){
	dwnet_drop();
    }
    while( l > 0 ){
	if( l > CHUNKZ )
	    s = CHUNKZ;
	else 
	    s = l;
	dwnet_recv_chunk( buf, s );
	buf += CHUNKZ;
	l -= CHUNKZ;
    }
    return packetz;
}

/* initialize network device */
int device_init( void )
{
    int ret;
    struct dw_trans d;
    unsigned char buf[2];

    fd=open( "/dev/dw0", O_RDONLY, 0 );
    if( fd < 0 ){
	return -1;
    }
    /* request lwwire extension "packet" */
    buf[0] = 0xf0;    // request extension
    buf[1] = 0x01;    // packet
    d.sbuf = buf;
    d.sbufz = 2;
    d.rbuf = buf;
    d.rbufz = 1;
    if ( ioctl( fd, DRIVEWIREC_TRANS, &d ) < 0 )
	return -1;
    if ( buf[0] != 0x42 )
	return -1;
    return 0;
}

uint8_t has_arp = 1;
uint16_t mtu = 1500;
