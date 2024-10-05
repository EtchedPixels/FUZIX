/* A program to get the time from drivewire.  Displays it or sets the
   system clock from it.  Must be super user
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/drivewire.h>


char *devname="/dev/dw0";

static const uint16_t mktime_moffset[12]= { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

int silent = 0; /* silent I/O failure flag */

static void printe( char *s ){
    write(2, s, strlen(s) );
    write(2, "\n", 1 );
}

static int get_time( uint8_t *tbuf )
{
    int fd;
    int ret;
    struct dw_trans d;

    fd = open( devname, O_RDONLY );
    if( fd < 1){
	perror( "drivewire device open");
	exit(1);
    }

    tbuf[0]=0x23;
    d.sbuf = tbuf;
    d.sbufz = 1;
    d.rbuf = tbuf;
    d.rbufz = 6;
    
    ret = ioctl( fd, DRIVEWIREC_TRANS, &d );
    if (ret)
        if (errno != EIO || !silent)
            perror("drivewire");
    close( fd );
    return ret;
}


int main( int argc, char *argv[] ){

    int i,x;
    unsigned char buf[6];
    uint8_t year, month, day, hour, minute, second;
    time_t ret;   /* accumulator for conversion */
    int setflg = 0;     /* set system time flag */
    int disflg = 0;     /* display retrieved time flag */
    int parbrk = 0;     /* parse break flag */

    /* scan args */
    for( x = 1; x < argc; x++ ){
	if( argv[x][0] != '-' ){
	    printe( "bad arg" );
	    exit(1);
	}
	for( i = 1; argv[x][i]; i++ ){
	    switch( argv[x][i] ){
	    case 's':
		setflg = 1;
		break;
	    case 'd':
		disflg = 1;
		break;
	    case 'q':
		silent = 1;
		break;
	    case 'x':
		devname = argv[++x];
		if( ! devname ){
		    printe("bad device name");
		    exit(1);
		}
		parbrk = 1;
		break;
	    default:
		printe("bad option" );
		exit(1);
	    }
	    if( parbrk ){
		parbrk = 0;
		break;
	    }
	}
    }

    if (get_time(buf))
	exit(1);

    /* figure out secs from epoc */
     
    year   = buf[0];
    month  = buf[1];
    day    = buf[2];
    hour   = buf[3];
    minute = buf[4];
    second = buf[5];
    
    if(year < 70)
	year += 100;
    
    /* following code is based on utc_mktime() from ELKS
       https://github.com/jbruchon/elks/blob/master/elkscmd/sh_utils/date.c 
    */
    
    /* uses zero-based month index */
    month--;
    
    /* calculate days from years */
    ret=365;
    ret *= year - 70;
    
    /* count leap days in preceding years */
    ret += (year - 69) >> 2;
    
    
    /* calculate days from months */
    ret += mktime_moffset[month];
    
    /* add in this year's leap day, if any */
    if (((year & 3) == 0) && (month > 1)) 
	ret++;
    
    /* add in days in this month */
    ret += day - 1;
    /* convert to hours */
    ret *= 24;
    ret += hour;
    
    /* convert to minutes */
    ret *= 60;
    ret += minute;
    
    /* convert to seconds */
    ret *= 60;
    ret += second;

    if( disflg || !setflg )
	fputs(ctime(&ret),stdout);

    if( setflg ){
	/* This is a sleezy cast */
	x=stime(&ret);
	if( x ){
	    perror( "stime" );
	    exit(1);
	}
    }

    exit(0);
    
}
