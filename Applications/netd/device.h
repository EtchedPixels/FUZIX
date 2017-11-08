/*

  Device Driver header for uip/fuzix


 */

/* implemented by device: */

/* send sbuf, size len to the device
   returns 0 on success, -1 on error
 */
int device_send( char *sbuf, int len );


/* get sbuf, max size len from the device 
   returns size of recieved packets, 0 if nothing waiting
*/
int device_read( char *buf, int len );



/* initialize device
   returns 0 on success, -1 on error
*/
int device_init( void );

/* set this is 1 if interface needs ARP */
extern uint8_t has_arp;
