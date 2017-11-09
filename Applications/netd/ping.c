/*
  A Cheesey ipv4 ping client

  (C) 2017, Brett M. Gordon, GPL2 under Fuzix

  todo:
  * add timestamp to packets to measure delta-t
  * add time max/min/avg/sdev stats like real ping
  * check for endian problems
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
#define SOCK_RAW    1

struct ip {
    uint8_t ver;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t off;
    uint8_t ttl;
    uint8_t proto;
    uint16_t cksum;
    uint32_t src;
    uint32_t dest;
};

struct icmp {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint16_t id;
    uint16_t seq;
};

char *data = "FUZIX ping client";

#define MAXBUF 256
int fd;
char buf[MAXBUF];
struct sockaddr_in addr;
int id;
int seq=0;
int sent=0;
int nrecv=0;

void alarm_handler( int signum ){
    return;
}

void int_handler( int signum ){
    printf("sent %d, recv %d, %d%%\n",
	   sent, nrecv, nrecv*100/sent );
    exit(0);
}

/* print a IP address */
void ipprint( uint32_t *a ){
    char *b = (char *)a;
    printf("%d.%d.%d.%d", b[0], b[1], b[2], b[3] );
}


/* returns inet chksum */
uint16_t cksum( char *b, int len ){
    uint16_t sum = 0;
    uint16_t t;
    char *e = b + len;
    b[len] = 0;
    while(b < e){
	t = (b[0] << 8) + b[1];
	sum += t;
	if(sum < t) sum++;
	b += 2;
    }
    return ~sum;
}


/* sends ping to remote */
void sendping( void ){
    struct icmp *i = (struct icmp *)buf;
    int l = strlen(data) + 8;
    memset( buf, 0, MAXBUF);
    i->type = 8;  // echo request
    i->id = htons(id);
    i->seq = htons(seq);
    strcpy( &buf[8], data );
    i->cksum = htons(cksum(buf, l));
    write(fd, buf, l);
    sent++;
    seq++;
}

my_open( int argc, char *argv[]){
    struct hostent *h;

    h=gethostbyname( argv[1] );
    if( ! h ){
	fprintf( stderr, "cannot resolve hostname\n" );
	exit(1);
    }
    memcpy( &addr.sin_addr.s_addr, h->h_addr_list[0], 4 );

    fd = socket(AF_INET, SOCK_RAW, 1);
    if (fd < 0) {
	perror("af_inet sock_stream 0");
	exit(1);
    }

    /* fuzix raw sockets (for now) repurposes the connect() 
       address struct's port no to pass it's protocol number
     */  
    addr.sin_port = 1;  
    addr.sin_family = AF_INET;
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	perror("connect");
	exit(1);
    }
}


int main( int argc, char *argv[] ){
    uint16_t c = 0;
    int x,i;
    time_t t;
    struct icmp *icmpbuf;
    struct ip *ipbuf;
    struct ip *ipbuf2;

    srand(time(&t));
    id = rand();

    signal(SIGINT, int_handler);

    if( argc < 2 ){
	fprintf( stderr,"usage: ping hostname\n");
	exit(1);
    }

    my_open( argc, argv );
    

    while(1){
	sendping();
	/* FIXME: this breaks if the alarm occurs before the read under
	   load - sigsetjmp/siglongjmp needed I think */
	signal( SIGALRM, alarm_handler );
	alarm(2);
    ragain:
	x=read( fd, buf, MAXBUF);
	if (x > 0){
	    ipbuf = (struct ip *)buf;
	    if (ipbuf->ver >> 4 != 4) {
	        printf("Not v4 ?\n");
		goto ragain;
            }
	    icmpbuf = (struct icmp *)(buf + (ipbuf->ver & 15) * 4);
	    /* check for dest unreachable icmp messages */
	    if ( icmpbuf->type == 3 ){
		/* point to original ip packet in icmp data field */
		ipbuf2 = (struct ip *)(icmpbuf + 1);
		icmpbuf = (struct icmp *)((char *)ipbuf2 + (ipbuf2->ver & 15) * 4);
		/* check the bombed-out ip packet to see if it's ours */
		if( icmpbuf->id == htons(id) ){
		    printf("ICMP: from ");
		    ipprint( &ipbuf->src );
		    printf(" dest unreachable\n");
		}
		goto ragain;
	    }	
	    /* filter for our id */
	    if( icmpbuf->id != htons(id) ) {
	        printf("Bad id %d\n", ntohs(icmpbuf->id));
		goto ragain;
            }
	    /* passed filters, so this must be one of our pings */
	    nrecv++;
	    printf("%d bytes from %s (", x, argv[1] );
	    ipprint( &ipbuf->src );
	    printf(") req=%d", ntohs(icmpbuf->seq));
	    printf("\n");
	}
	sleep(1);
    }
}
