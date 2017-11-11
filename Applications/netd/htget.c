#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "netdb.h"


static int sock;
static char buf[512];
static char *bufend = buf;
static char *readp = buf;
static int buflen = 0;

void writes(int fd, const char *p)
{
    write(fd, p, strlen(p));
}

void xflush(void)
{
    if (write(sock, buf, buflen) != buflen) {
        perror("write");
        exit(1);
    }
    buflen = 0;
}

void xwrites(const char *p)
{
    int l = strlen(p);
    if (l + buflen > 512)
        xflush();
    memcpy(buf + buflen, p, l);
    buflen += l;
}

int xread(void)
{
    int len = read(sock, buf, 512);
    if (len < 0) {
        perror("read");
        exit(1);
    }
    return len;
}

int xreadline(void)
{
    int len;

    if (readp != buf && bufend > readp) {
        memcpy(buf, readp, bufend - readp);
        bufend -= (readp - buf);
    }
    readp = buf;

    len = read(sock, buf + buflen, 512 - buflen);
    if (len < 0) {
        perror("read");
        exit(1);
    }
    buflen += len;
    bufend += len;

    while(readp < bufend) {
        if (*readp == '\r' && readp[1] == '\n') {
            *readp++ = '\n';
            *readp++ = 0;
            len = readp - buf;
            buflen -= len;
            return len;
        }
        readp++;
    }
    writes(2,"htpget: overlong/misformatted header\n");
    exit(1);
}
    

int main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct hostent *h;
    uint16_t port = 80;
    char *pp;
    char *fp;
    int of = 1;
    int code;
    int len;
    uint8_t looped = 0;

    if (argc != 2) {
        writes(2, "htget url\n");
        exit(1);
    }
    if (strncmp(argv[1], "http://", 7)) {
        writes(2, "htget: only http:// is supported.\n");
        exit(2);
    }
    argv[1] += 7;
    pp = strrchr(argv[1], ':');
    if (pp) {
        *pp++ = 0;
        port = atoi(pp);
        if (port == 0) {
            writes(2, "htget: invalid port\n");
            exit(1);
        }
    }

    fp = strchr(argv[1], '/');
    if (fp)
        *fp = 0;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    
    h = gethostbyname(argv[1]);
    if (h == NULL) {
        writes(2, "htget: unknown host \"");
        writes(2, argv[1]);
        writes(2, "\"\n");
        exit(1);
    }
    memcpy( &sin.sin_addr.s_addr, h->h_addr_list[0], 4 );

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }
    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("connect");
        exit(1);
    }

    xwrites("GET /");
    if (fp)
        xwrites(fp);
    xwrites(" HTTP/1.1\r\n");
    xwrites("Host: ");
    xwrites(argv[1]);
    if (pp) {
        xwrites(":");
        xwrites(pp);
    }
    xwrites("\r\nUser-Agent: Fuzix-htget/0.1\r\nConnection: close\r\n\r\n");
    xflush();

    do {
        xreadline();
        errno = 0;
        pp = strchr(buf, ' ');
        if (pp == NULL) {
            writes(2, "htget: invalid reply\n");
            writes(2, buf);
            exit(1);
        }
        pp++;
        code = strtoul(pp, &pp, 10);
        if (code < 100 || *pp++ != ' ') {
            writes(2, "htget: invalid reply\n");
            writes(2, buf);
            exit(1);
        }

        if (code  != 200)
            writes(2, buf);
        do {
            xreadline();
            if (code != 200)
                writes(2, buf);
        } while(*buf != '\n');

        /* A 100 code means "I'm thinking please wait then a header cycle then
           a real header and has a blank line following */
        if (code == 100)
            xreadline();
    } while (code == 100 && !looped++);
    /* FIXME: if we saw a Transfer-Encoding: chunked" we need to do this
       bit differently */
    if (code == 200) {
	if (write(of, readp, bufend - readp) < 0){
	    perror("write");
	    exit(1);
	}
        while((len = xread()) > 0) {
            if (write(of, buf, len) != len) {
                perror("write");
                exit(1);
            }
        }
    }
    return 0;
}

    
            
