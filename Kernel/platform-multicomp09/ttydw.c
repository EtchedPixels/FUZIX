/* Drivewire TTY's
   
   config.h defines:

   DW_VSER_NUM - number of virtual serial ports ( 0-14 )
   DW_VWIN_NUM - number of virtual "9server" windows ( 0-14 )
   DW_MIN_OFF  - tty device minor offset 
   
   example:

   #define DW_VSER_NUM 4    
   #define DW_VWIN_NUM 4     
   #define DW_MIN_OFF  3    
   #define NUM_DEV_TTY 10

   then this module will map drivewire tty's to the following minors:

   minor no    description
   0-2         not mapped.
   3-6         Drivewire Virtual Serial Ports 0-3
   7-10        Drivewire Virtual Window Ports 0-3

   Modify your platform's devtty.c to call the following external
   functions: (don't forget to include ttydw.h)

   Make sure to up your NUM_DEV_TTY define in your config.h to cover
   all the added tty's !

   External/Public Functions:

   void dw_val( uint8_t minor );
   void dw_putc( uint8_t minor, unsigned char c );
   void dw_vopen( uint8_t minor );
   void dw_vclose( uint8_t minor );
   int dw_carrier( uint8_t minor );
   void dw_vpoll( );

   How you switch to these driver function/methods from your devtty.c
   is up to you...

   dw_vpoll should be called from the timer interrupt.  To conserve
   cpu time, this fuction only actually polls the DW server every .25
   seconds

   Finally, you will have to provide a platform dependent
   "dw_transaction" routine/function:

   int16_t dw_transaction( char *send, uint16_t scnt,
                            char *recv, uint16_t rcnt, uint8_t rawf )

   where "send" is a data buffer to send
         "scnt" is the size of the send buffer
         "recv" is a data buffer for the received reply
	 "rcnt" is the size of the recv buffer
	 "rawf" rd/wr drirectly to userspace
   returns:  0 on no error
            -1 on DW reception framing error (too slow!!)
            -2 on not all bytes received


   Usage Notes:
   
   You cannot stick a "sh" or "getty" on Virtual Serial ports
   directly, as these Drivewire ports speak a higher level, but
   simple, API for do TCP, DW control, MIDI etc.  However, the Window
   ports, are just that... you can "sh", "getty", or whatever them
   directly. (stick 'em in your inittab)

   Unfortunately, the current Drivewire server does not issue a close
   port upon the closing of the actual window!  It will, however, open
   them on you server's machine upon Fuzix's opening of the port.

   A stock DriveWire4 installation sets up the Virtual Serial Ports like this:

   ports 1-13 : generic ports
   ports 14 : direct connection to DriveWire's General MIDI
*/

#include <kernel.h>
#include <printf.h>
#include <tty.h>
#include <devdw.h>

#define DW_FASTWRITE 0x80
#define DW_SETSTAT   0xC4
#define DW_SERREAD   0x43
#define DW_SERREADM  0x63
#define DW_INIT      0x5a

#define DW_VOPEN     0x29
#define DW_VCLOSE    0x2A

#define DW_NS_OFF    ( DW_MIN_OFF + DW_VSER_NUM )


/* Internal Structure to represent state of DW ports */
struct dw_in{
	uint8_t flags;  /* flags for port */
	/* outgoing buffer here! */
};

/* port flags */
#define DW_FLG_OPEN 1     /* is port open? */


/* and a table of the above structures */
struct dw_in dwtab[ DW_VSER_NUM + DW_VWIN_NUM ];

/* How many vsync ticks to wait until polling again, if
   DW reports no data is waiting. */
#define MAX_WAIT     TICKSPERSEC / 4

int wait=MAX_WAIT;

/* Number of ports open. IF zero then polling routine
   will not poll */
int open_ports=0;


/* buffer for receiving multiple bytes from vport channels */
unsigned char tbuf[256];


int mini( int a, int b ){
	if( a < b ) return a;
	return b;
}


/* Gets dw_tab entry for given minor */
struct dw_in *dw_gettab( uint8_t minor ){
	return &dwtab[ minor - DW_MIN_OFF ] ;
}

/* Translates a DW port no. to a proper minor no */
int dw_minor( uint8_t port ){
	if( port >= 16 ) return port - 16 + DW_NS_OFF  ;
	int ret = port + DW_MIN_OFF - 1 ;
	return ret;
					
}


/* Translates a Minor to a port no */
int dw_port( uint8_t minor ){
	int ret = minor - DW_MIN_OFF + 1;
	if( minor >= DW_NS_OFF ) 
		return 	minor + 16 - DW_NS_OFF ;
	return ret;
}



/* Put a character to the DriveWire port */
void dw_putc( uint8_t minor, unsigned char c ){
	unsigned char buf[2];
	buf[0]=DW_FASTWRITE | dw_port( minor ) ;
	buf[1]=c;
	dw_transaction( buf, 2, NULL, 0, 0 );
}



/* Open a DriveWire port */
void dw_vopen( uint8_t minor ){
	struct dw_in *p=dw_gettab( minor );
	unsigned char buf[3];
	buf[0]=DW_SETSTAT;
	buf[1]=dw_port( minor );
	buf[2]=DW_VOPEN;
	if( ! ( p->flags & DW_FLG_OPEN ) ){
		dw_transaction( buf, 3, NULL, 0, 0 );
		open_ports++;
	}
	p->flags |= DW_FLG_OPEN;
}

/* Close a DriveWire port */
void dw_vclose( uint8_t minor){
	struct dw_in *p=dw_gettab( minor );
	unsigned char buf[3];
	buf[0]=DW_SETSTAT;
	buf[1]=dw_port( minor );
	buf[2]=DW_VCLOSE;
	if( p->flags & DW_FLG_OPEN ){
		dw_transaction( buf, 3, NULL, 0, 0 );
	}
}



/* Return number of byte in tty's input queue */
int qfree( uint8_t minor ){
	queue_t *q = &ttyinq[minor];
	return q->q_size - q->q_count;
}



/* Poll and add chars (if any) to input q
 */
void dw_vpoll( ){
	unsigned char buf[2];
	int i;
	/* don't waste time polling of no ports are open*/
	if( ! open_ports ) return ;
	/* check ticks - don't poll until our delay is done */
	if( --wait ) return;
	/* up to four transactions at a poll */
	for( i=0; i<4; i++){
		buf[0]=DW_SERREAD;
		dw_transaction( buf, 1, buf, 2, 0 );
		/* nothing waiting ? */
		if( ! (buf[0] & 0x7f) ) {
			wait=MAX_WAIT;
			break;
		}
		/* VSER Channel single datum */
		if( buf[0]<16 ){
			int minor=dw_minor( buf[0] - 1 );
			tty_inproc( minor, buf[1] );
			continue;
		}
		/* VSER Channel closed? */
		if( buf[0] == 16 ){
			int minor=dw_minor( buf[1] );
			struct dw_in *p=dw_gettab( minor );
		       	if( p->flags & DW_FLG_OPEN ){
				p->flags &= ~DW_FLG_OPEN;
				open_ports--;
				if( ttydata[minor].users )
					tty_carrier_drop( minor);
			}
			continue;
		}
		/* VSER channel multiple data */
		if( buf[0] < 32 ){
			int i;
			unsigned char b[3];
			int min;
			int minor=dw_minor( buf[0]-17 );
			b[0]=DW_SERREADM;
			b[1]=buf[0]-17;
			min=mini( buf[1], qfree( minor ) );
			b[2]=min;
			if( !min ){
				wait=1;
				break;
			}
			dw_transaction( b,3,tbuf, min, 0 );
			for( i=0; i<min; i++){
				tty_inproc( minor, tbuf[i] );
			}
			wait=1;
			break;
		}
		/* VWIN channel single datum */
		if( buf[0] < 144 ){
			int minor=dw_minor( buf[0]-48 );
			tty_inproc( minor, buf[1] );
			continue;
		}
		/* something we don't handle? */
		kprintf("out of band data\n");
	}
}



/* Tests DriveWire port for being open */
/*   returns: 1 on open, 0 on closed */
int dw_carrier( uint8_t minor ){
	struct dw_in *p=dw_gettab( minor );
	return p->flags & DW_FLG_OPEN ;
}


/* (re) Initializes DW */
void dw_init( ){
	unsigned char buf[2];
	buf[0]=DW_INIT;
	buf[1]=0x42;
	dw_transaction( buf,2,buf,1,0 );
}
