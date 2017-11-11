/*
 * 	SLIP interface. Really we need select() support for this or to ruute
 *	slip via the kernel. TODO
 */
  
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "device.h"

static int fd;  /* fd of the tty */

static char ibuf[297];
static char *iptr = ibuf;

#define SLIP_END		0xC0
#define SLIP_ESC		0xDB
#define ESC_END			0xDC
#define ESC_ESC			0xDD

static void slip_outbuf(uint8_t c)
{
    /* FIXME: buffer.. ? */
    write(fd, &c, 1);
}

static void slip_flush(void)
{
}

static void slip_out(uint8_t c)
{
    if (c == SLIP_ESC) {
        slip_outbuf(SLIP_ESC);
        slip_outbuf(ESC_ESC);
    } else if (c == SLIP_END) {
        slip_outbuf(SLIP_ESC);
        slip_outbuf(ESC_END);
    } else
        slip_outbuf(c);
}

static void slip_begin(void)
{
    /* Should send an end mark if idle for a bit */
    slip_outbuf(SLIP_END);
}

static void slip_end(void)
{
    slip_outbuf(SLIP_END);
    slip_flush();
}

static void badpacket(void)
{
    iptr = ibuf;
    fprintf(stderr, "overlong frame\n");
}

static char scratch[256];

/* returns number of byte in next packet, 0 = nothing waiting */
static int slip_poll( void )
{
    static uint8_t esc = 0;
    static uint8_t *p = scratch;
    static int l = 0;
    int n;
    uint8_t c;

    if (scratch != p && l)
        memmove(scratch, p, l);
    p = scratch;

    if (l < 192) {
        /* Ok so this should be a ring buffer FIXME */
        n = read(fd, scratch + l, 256 - l);
        if (n == -1) {
            if (errno != EAGAIN) {
                fprintf(stderr, "port failed\n");
                exit(1);
            }
            n = 0;
        }
        l += n;
        if (l == 0)
            return 0;
    }
    while(l--) {
        /* If we saw an escape then process the escapes */
        c = *p++;
        if (esc) {
            esc = 0;
            if (c == ESC_END)
                *iptr++ = SLIP_END;
            else if (c == ESC_ESC)
                *iptr++ = SLIP_ESC;
            else {
                badpacket();
                continue;
            }
        } else {
            /* An escape begins */
            if (c == SLIP_ESC) {
                esc = 1;
                continue;
            }
            /* End of frame */
            if (c == SLIP_END) { 
                int len = iptr - ibuf;
                if (len == 0)
                    continue;
                return len;
            }
            /* Queue the byte */
            *iptr++ = c;
        }
        if (iptr == ibuf + 297)
            badpacket();
    }
    l = 0;
    return 0;
}

/* receive packet to a buffer */
static int slip_recv( unsigned char *b, int len )
{
    len = iptr - ibuf;
    /* Mark it as IP */
    b[12] = 0x08;
    b[13] = 0x00;
    memcpy(b+14, ibuf, len);
    iptr = ibuf;
    return len;
}

/* send packet to net device
   We should do this asynchronously to receive ideally */
int device_send( char *sbuf, int len )
{
    sbuf += 14;	/* Don't send a mac header */
    slip_begin();
    while(len--)
        slip_out(*sbuf++);
    slip_end();
    return 0;
}

int device_read( char *buf, int len )
{
    if (slip_poll() == 0)
        return -1;
    return slip_recv(buf, len);
}

static struct termios t;
/* initialize network device */
int device_init(void)
{
    /* FIXME: don't hard code */
    fd=open( "/dev/tty4", O_RDWR|O_NOCTTY);
    if( fd < 0  || tcgetattr(fd, &t) < 0) {
        perror("/dev/tty4");
	return -1;
    }
    t.c_iflag  = IGNBRK;
    t.c_oflag = 0;
    t.c_cflag &= ~(CSIZE|PARENB|HUPCL);
    t.c_cflag |= CREAD|CS8;
    t.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE|ECHOK);
    memset(t.c_cc, 0, NCCS);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 3;	/* 0.3 seconds */

    if (tcsetattr(fd, 0, &t) < 0) {
        perror("/dev/tty4");
        return -1;
    }
    return 0;
}

uint8_t has_arp = 0;
uint16_t mtu = 296;
