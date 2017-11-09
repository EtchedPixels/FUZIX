/*
  A Cheesey NTP client to set system time

  (C) 2017, Brett M. Gordon, GPL2 under Fuzix

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
#include <getopt.h>
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
int setflg = 0;
int disflg = 0;
int port = 123;  /* default port no */

void alarm_handler( int signum ){
    return;
}

void pusage( void ){
    fprintf(stderr, "ntpdate -sd [-o tz] server\n");
    exit(1);
}

/* sends query to remote */
void sendq( void ){
    struct ntp_t *i = (struct ntp_t *)buf;
    memset( buf, 0, MAXBUF );
    i->lvm = 0xe3;
    write(fd, buf, 48);
}

my_open( int argc, char *argv[]){
    struct hostent *h;

    h=gethostbyname( argv[optind] );
    if (!h){
	fprintf( stderr, "cannot resolve hostname\n" );
	exit(1);
    }
    memcpy( &addr.sin_addr.s_addr, h->h_addr_list[0], 4 );

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
	perror("af_inet sock_dgram 0");
	exit(1);
    }

    addr.sin_port = port;
    addr.sin_family = AF_INET;
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	perror("connect");
	exit(1);
    }
}


int main( int argc, char *argv[] ){
    int retries;
    int rv;
    uint32_t uv = 0;
    int tz = 0;
    struct ntp_t *ptr = (struct ntp_t *)buf;
    char *p;

    while ((rv = getopt( argc, argv, "p:o:ds" )) > 0 ){
	switch (rv){
	case 'p':
	    port = atoi( optarg );
	    break;
	case 'o':
	    tz = atoi( optarg );
	    if (tz < -12 || tz > 12){
		fprintf(stderr, "bad timezone\n");
		exit(1);
	    }
	    break;
	case 's':
	    setflg = 1;
	    break;
	case 'd':
	    disflg = 1;
	    break;
	case '?':
	    pusage();
	}
    }
    if( ! argv[optind] )
	pusage();

    my_open( argc, argv );

    retries = 3;
    while (retries--){
	sendq();
	signal( SIGALRM, alarm_handler );
	alarm(2);
	rv = read( fd, buf, MAXBUF);
	if (rv < 0 )
	    continue;
	if (rv >= sizeof( struct ntp_t ))
	    goto process;

    }
    fprintf(stderr, "timeout\n");
    exit(1);

 process:

    uv = ptr->xmit.sec;
    uv -= 2208988800L;
    uv += tz * 60 * 60;

    if (disflg || !setflg)
	printf(ctime((time_t *)&uv));

    if (setflg){
	rv = stime((time_t *)&uv);
	if (rv){
	    perror( "stime" );
	    exit(1);
	}
    }
    exit(0);
}
