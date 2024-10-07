/*
  A Cheesey Dig Client

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "netdb.h"

struct header{
    uint16_t id;
    uint8_t  cntl;
    uint8_t  ret;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

/* This needs to be packed if your compiler adds padding */
struct RRtail{
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlen;
};


int fd;
char buf[1024];
char server[17] = "192.168.1.1";
char name[256] = ".";

void alarm_handler( int signum ){
    return;
}


int send_question( char *name ){
    struct header *p=( struct header *)buf;
    struct RRtail *t;
    
    char *i = name;
    char *o = buf + sizeof(struct header);
    char *l = o++;

    memset( p, 0, sizeof(buf) );
    p->id = htons(42);     /* "random" query ID */
    p->cntl = 0x1;  /* request a recursive query */
    p->qdcount = htons(1); /* one question */
    /* fill out name string */
    
    while(1){
	if( ! *i )
	    break;
	if( *i == '.' ){
	    *l = o - l - 1 ;
	    i++;
	    l = o++;
	}
	else
	    *o++ = *i++;
    }
    *l = o - l - 1;
    *o++ = 0;
    /* fill out rest of RR */
    t = (struct RRtail *)o;
    /* type: A record */
    t->type = htons(1);
    /* class: inet */
    t->class = htons(1);
    o += 4;
    
    write( fd, buf, (int)(o - buf) );
    return 0;
}

void print_name( char *ptr ){
    while(1){
	if( *ptr == 0 ){
	    break;
	}
	if( (*ptr & 0xc0) == 0xc0 ){
	    ptr = buf + *(ptr + 1);
	    continue;
	}
	fwrite( ptr + 1, *ptr, 1, stdout );
	//	write(1, ptr + 1, *ptr );
	printf(".");
	ptr += *ptr + 1;
    }
    printf("\t\t");
    return;
}


void print_entry( char **pptr, int no ){
    struct RRtail *t;
    int i,j;
    char *ptr = *pptr;
    struct in_addr addr;
    for( i = 0; i < no; i++ ){
	print_name( ptr );
	while(1){
	    if( *ptr == 0 ){
		ptr++;
		break;
	    }
	    if( (*ptr & 0xc0) == 0xc0 ){
		ptr += 2;
		break;
	    }
	    ptr += *ptr + 1;
	}
	
	t = (struct RRtail *)ptr;
	ptr += sizeof( struct RRtail);

	/* cname */
	if( ntohs(t->type) == 6 ){
	    printf( "SOA   ");
	    print_name( ptr );
	    ptr += ntohs(t->rdlen);
	}
	else if( ntohs(t->type) == 5 ){
	    printf( "CNAME ");
	    print_name( ptr );
	    ptr += ntohs(t->rdlen);
	}
	else if( ntohs(t->type) == 1 ){
	    printf( "A     ");
            memcpy(&addr, ptr, sizeof(struct in_addr));
            printf("%s", inet_ntoa(addr));
	    ptr += ntohs(t->rdlen);
	}
	else{
	    printf( "???   ");
	    ptr += ntohs(t->rdlen);
	}
	printf("\n");
    }
    *pptr = ptr;
}

/* open up /etc/resolv.conf and read in
   fixme: handles only small config files < 512 bytes */
int readrc( void ){
    char *ws = " \f\n\r\t\v";
    char *ptr;
    int x;

    fd = open( "/etc/resolv.conf", O_RDONLY );
    if ( fd < 0 ){
	return -1;
    }
    x = read( fd, buf, 511 );
    if( x < 1 )
	goto reterr;
    buf[x] = 0;
    ptr = strtok( buf, ws );
    while ( ptr ){
	if ( ! strcmp( ptr, "nameserver" ) ){
	    ptr = strtok( NULL, ws );
	    if( ptr == NULL )
		goto reterr;
	    strcpy( server, ptr );
	    goto retok;
	}
	ptr = strtok( NULL, ws );
    }
 reterr:
    close(fd);
    return -1;
 retok:
    close(fd);
    return 0;
}

int main( int argc, char *argv[] ){

    struct sockaddr_in addr;
    int x;
    int tries = 5;

    readrc();

    for( x = 1; x < argc; x++ ){
	if( argv[x][0] == '@' )
	    strncpy( server, &(argv[x][1]), 16);
	else
	    strncpy( name, argv[x], 256 );
    }

    fd = socket( AF_INET, SOCK_DGRAM, 0);
    if( fd < 0 ){
	perror("socket");
	exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons( 53 );
    inet_pton( AF_INET, server, &addr.sin_addr.s_addr );

    if( connect( fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ){
	perror("connect");
	exit(1);
    }

    for( ; tries ; tries-- ){
	send_question( name );
	signal( SIGALRM, alarm_handler );
	alarm(2);
	x = read( fd, buf, 1024 );
	if( x <= 0 )
	    continue;
	goto process;
    }
    perror("timeout");
    exit(1);

 process:
    /* process packet */
    {
	struct header *h = (struct header *)buf;

	char *ptr;
	int i;
	
	if( ntohs(h->id) != 42 ){
	    fprintf( stderr, "bad ID\n");
	    exit(1);
	}

	if(!(h->cntl & 0x80)){
	    fprintf( stderr, "not an answer\n");
	    exit(1);
	}

	if(!(h->ret & 0x80)) {
	    fprintf( stderr, "server doesn't do recursion\n");
	}
	/* skip over our question(s) */
	printf("qust: %d, ans: %d, ns: %d, add: %d\n",
	       ntohs(h->qdcount), ntohs(h->ancount), ntohs(h->nscount), ntohs(h->arcount) );
	ptr = buf + sizeof( struct header );
	printf("questions:\n");
	/* move ptr past questions */
	for( i = 0; i < ntohs(h->qdcount); i++ ){
	    print_name( ptr );
	    printf("\n");
	    while(1){
		if( *ptr == 0 ){
		    ptr++;
		    break;
		}
		if( (*ptr & 0xc0) == 0xc0 ){
		    ptr += 2;
		    break;
		}
		ptr += *ptr + 1;
	    }
	    ptr +=4;
	}   
	/* print out answers */
	if( ntohs(h->ancount) ){
	    printf("answers:\n");
	    print_entry( &ptr, ntohs(h->ancount) );
	}
	if( ntohs(h->nscount) ){
	    printf("authority:\n");
	    print_entry( &ptr, ntohs(h->nscount) );
	}
	if( ntohs(h->arcount) ){
	    printf("additional:\n");
	    print_entry( &ptr, ntohs(h->arcount) );
	}
    }
    exit(0);
}
