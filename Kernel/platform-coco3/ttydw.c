/* Drivewire tty
        This module needs some help to make this cross-platform.
	And, it only handles one virtual Window.
 */

#include <kernel.h>
#include <printf.h>
#include <tty.h>
#include <devdw.h>

#define DW_FASTWRITE 0x80
#define DW_SETSTAT   0xC4
#define DW_SERREAD   0x43

#define DW_VOPEN     0x29
#define DW_VCLOSE    0x2A

/* How many vsync ticks to wait until polling again, if
   DW reports no data is waiting. */
#define MAX_WAIT     TICKSPERSEC / 4

int wait=MAX_WAIT;

/* Number of ports open. IF zero then polling routine
   will not poll */
int open_ports=0;

void dw_putc( uint8_t minor, unsigned char c ){
  unsigned char buf[2];
  buf[0]=DW_FASTWRITE | (16 + minor - 3) ;
  buf[1]=c;
  dw_transaction( buf, 2, NULL, 0 );
  if( c == '\n' ){
    c='\r';
    dw_transaction( buf,2, NULL, 0 );
  }
}


void dw_vopen( uint8_t minor ){
  unsigned char buf[3];
  buf[0]=DW_SETSTAT;
  buf[1]=minor-3+16;
  buf[2]=DW_VOPEN;
  dw_transaction( buf, 3, NULL, 0 );
  open_ports++;
}


void dw_vclose( uint8_t minor){
  unsigned char buf[3];
  buf[0]=DW_SETSTAT;
  buf[1]=minor-3+16;
  buf[2]=DW_VCLOSE;
  dw_transaction( buf, 3, NULL, 0 );
  open_ports--;
}

/* Poll and add chars (if any) to input q
   no multple char reads yet
 */
void dw_vpoll( ){
  unsigned char buf[2];
  int i;
  /* don't waste time polling of no port are open*/
  if( ! open_ports ) return ;
  /* check ticks */
  if( --wait ) return;
  for( i=0; i<4; i++){
    buf[0]=DW_SERREAD;
    dw_transaction( buf, 1, buf, 2 );
    // nothing waiting ?
    if( ! (buf[0] & 0x7e) ) {
      wait=MAX_WAIT;
      break;
    }
    // Window data?
    if( buf[0] & 0x7e ){
      int minor=buf[0]-64+3;
      if( buf[1]== '\r' ) buf[1]='\n';
      tty_inproc( minor, buf[1] );
      continue;
    }
    kprintf("out of band data\n");
  }
}
