/* 

   A simple http server

 */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct sockaddr_in addr;
struct sockaddr_in laddr;
int lfd;


void my_open( int argc, char *argv[]){
    int port = 8080;    /* default port */

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
	perror("af_inet sock_stream 0");
	exit(1);
    }
    if( bind( lfd, (struct sockaddr *)&addr, sizeof(addr) ) ){
	perror("bind");
	exit(1);
    }

    if( listen( lfd, 1 ) ){
	perror("listen");
	exit(1);
    }
    if (accept(lfd, NULL, NULL) < 0)
     perror("accept");
}

int main( int argc, char *argv[]){
	int len;

	my_open( argc, argv );
	printf("httpd server\n");
	return 0;


}


