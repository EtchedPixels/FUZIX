/*
  This is a net_native device driver for Jim Brain's CoCoNIC card. This
  card is based on the CS8900A, and Jim's design uses the "I/O Memory Access",
  mapping the NIC's 16 bytes of access regs to 0xff40.

  note: This plays with I/O memory, of course, and shouldn't be in userspace!!!

  TODO: dedicated polling routine for in-kernel operation

*/

#include <types.h>
#include <stdio.h>
#include "device.h"
#include "uip.h"


#define rxtx     *(volatile uint16_t *)0xff60 /* rxrx data */
#define rxtx8low *(volatile uint8_t *)0xff60 /* Low byte of rxtx data */
#define rxtx8hi  *(volatile uint8_t *)0xff61 /* high byte of rxtx data */
#define txcmd  *(volatile uint16_t *)0xff64 /* transmit command register */
#define txlen  *(volatile uint16_t *)0xff66 /* transmit length register */
#define pageptr  *(volatile uint16_t *)0xff6a  /* packetpage pointer register */
#define pagedata *(volatile uint16_t *)0xff6c /* packetpage data register */
#define page8low *(volatile uint8_t *)0xff6c /* packetpage data register */
#define page8hi  *(volatile uint8_t *)0xff6d /* packetpage data register */

#define cardctl *(volatile uint8_t *)0xff80 /* cntl reg (change address) */


/* swap bytes in word */
#define bswap( v )  ( v << 8 ) | ( v >> 8 )


/* Get 16 bit word from NIC */
static uint16_t getpp( uint16_t addr )
{
    /*
    pageptr = bswap( addr );
    return bswap( pagedata );
    */
    pageptr = bswap( addr );
    return ( page8hi << 8 ) | page8low;
}

/* set 16 bit word to packetpage */
static void setpp( uint16_t addr, uint16_t data ){
    pageptr = bswap( addr );
    pagedata =  bswap( data );
}

/* get status / length */
static uint16_t getlen( void )
{
    return (rxtx8hi << 8 ) | rxtx8low ;
}

/* drop a frame */
static void drop( void )
{
    setpp( 0x102, getpp( 0x102 ) | 0x40 );
}




/* send sbuf, size len to the device
   returns 0 on success, -1 on error
 */
int device_send( char *sbuf, int len )
{
    int alen;
    uint16_t *ptr = (uint16_t *)sbuf;

    /* send transmit command, length */
    txcmd = bswap( 0x00c0 );
    txlen = bswap( len );
    /* round up for word acesses to NIC */
    alen = (len + 1) / 2;
    /* wait for room in NIC, drop received frames if necessary */
    while( ! (getpp( 0x138 ) & 0x0100) )
	drop();
    /* send words from frame to NIC */
    while( alen-- ){
	rxtx = *ptr++;
    }
    return 0;
}


/* get sbuf, max size len from the device 
   returns size of recieved packets, 0 if nothing waiting
*/
int device_read( char *buf, int len )
{
    int rlen, alen;
    uint16_t *ptr = (uint16_t *)buf;

    /* return if there is no packet waiting for us */
    if( ! (getpp( 0x124 ) & 0xd00) )
	return 0;
    /* drop status */
    rlen = getlen();
    /* get length */
    rlen = getlen();
    /* word access round-up */
    alen = (rlen + 1) / 2;
    /* packet too big for our buffer?*/
    if( alen*2 > len ){
	drop();
	return 0;
    }
    /* get words from NIC into buffer */
    while( alen-- ){
	*ptr++ = rxtx;
    }
    return rlen;
}



/* initialize device
   returns 0 on success, -1 on error
*/
int device_init( void )
{
    /* change card's address */
    cardctl = 0x55;
    cardctl = 0xaa;
    cardctl = 0x22;
    cardctl = 0x01;
    cardctl = 0x60;

    /* check for card */
    uint16_t id = getpp( 0x0000 );
    if( id != 0x630e ){
	fprintf( stderr, "CoCoNIC card not found\n", id );
	return -1;
    }
    /* set MAC address of card */
    pageptr = bswap( 0x158 );
    page8low = uip_lladdr.addr[0];
    page8hi  = uip_lladdr.addr[1];

    pageptr = bswap( 0x15a );
    page8low = uip_lladdr.addr[2];
    page8hi  = uip_lladdr.addr[3];

    pageptr = bswap( 0x15c );
    page8low = uip_lladdr.addr[4];
    page8hi  = uip_lladdr.addr[5];

    /* turn on receiver / transmitter */
    setpp( 0x112, 0x00d3 );
    /* allow reception of our address frames and broadcasts */
    setpp( 0x104, 0x0d05 );

    return 0;
}


uint8_t has_arp = 1;
uint16_t mtu = 1500;
