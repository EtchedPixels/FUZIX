/* dw.c - Drivewire command

 */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>


char ibuf[128];
char obuf[128];
struct termios prior,new;



void puterr( char *str ){
	write(2, str, strlen(str));
}

void putstr( char *str ){
	write(1, str, strlen(str));
}

int main( int argc, char *argv[]){
	int fddw;
	int len;

	putstr( "DriveWire Terminal.  Type \"exit\" to quit.\n");

	while(1){
		putstr("dw> " );
		len=read(0, ibuf, 127 );
		ibuf[len]='\0';

		if( ! strncmp("exit", ibuf,4 ) ) exit(0);

		fddw=open("/dev/tty3", O_RDWR);

		if( ! fddw ){
			puterr( "Cannot open device\n" );
			exit(-1);
		}

		tcgetattr( fddw, &prior );
		tcgetattr( fddw, &new );
		new.c_lflag &= ~ECHO;
		new.c_cflag |= HUPCL;
		new.c_iflag |= IGNCR;
		tcsetattr( fddw, TCSANOW, &new );
	
		write( fddw, "dw ", 3);
		write( fddw, ibuf, len );
		while(1){
			len=read( fddw, obuf, 128 );
			if( len<=0 ){
				tcsetattr( fddw, TCSANOW, &prior );
				close( fddw );
				break;
			}
			write( 1, obuf, len );
		}
	}
}
