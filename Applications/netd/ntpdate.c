/*
  A Cheesey NTP client to set system time

  (C) 2017, Brett M. Gordon, GPL2 under Fuzix

  todo:
    * add port no argument
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include "netdb.h"

#define AF_INET     1
#define SOCK_DGRAM  2

typedef struct {
    uint32_t sec;
    uint32_t frac;
} ntime;

struct ntp_t {
    uint8_t lvm;
    uint8_t statum;
    uint8_t poll;
    uint8_t prec;
    uint32_t delay;
    uint32_t disp;
    uint32_t refid;
    ntime ref;
    ntime org;
    ntime rec;
    ntime xmit;
};


#define MAXBUF 256
int fd;
char buf[MAXBUF];
struct sockaddr_in addr;


void alarm_handler( int signum ){
    return;
}



/* sends query to remote */
void send( void ){
    memset( buf, 0, MAXBUF );
    struct ntp_t *i = (struct ntp_t *)buf;
    i->lvm = 0xe3;
    write(fd, buf, 48);
}

my_open( int argc, char *argv[]){
    struct hostent *h;

    h=gethostbyname( argv[1] );
    if( ! h ){
	fprintf( stderr, "cannot resolve hostname\n" );
	exit(1);
    }
    memcpy( &addr.sin_addr.s_addr, h->h_addr_list[0], 4 );

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
	perror("af_inet sock_stream 0");
	exit(1);
    }

    /* fuzix raw sockets (for now) repurposes the connect() 
       address struct's port no to pass it's protocol number
     */  
    addr.sin_port = 123;  
    addr.sin_family = AF_INET;
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	perror("connect");
	exit(1);
    }
}


int main( int argc, char *argv[] ){
    int retries;
    int rv;
    
    if( argc < 2 ){
	fprintf( stderr,"usage: ntpdate hostname\n");
	exit(1);
    }

    my_open( argc, argv );
    
    retries = 5;
    while( retries--){
	send();
	signal( SIGALRM, alarm_handler );
	alarm(2);
	rv=read( fd, buf, MAXBUF);
	if (rv >= sizeof( struct ntp_t ))
	    goto process;
    }
    fprintf(stderr, "timeout");
    exit(1);

 process:
    {
	int i = 48;
	while( i-- ){
	    printf("%02x ", buf[i] );
	}
	printf("\n");
    }
    exit(0);

}
