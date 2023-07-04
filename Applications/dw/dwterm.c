/* dwtelnet - Drivewire - raw Telnet
 */
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>


#define TESC 0x19
#define ESC 0x1b


char ibuf[256];
int opos=0;

int flags;
struct termios prior,new;
struct termios lprior,lnew;
int fddw;
int para[4]={0,0,0,0};
int no=0;
char colors[8]={
	'0', '2', '4', '6',
	'1', '3', '5', '7'
};

int linemode=0;
int lecho=-1;
int pflag=0;
int hex=0;
int del=0;
int crnl=-1;
int fc=7;
int bc=0;
int inten=0;


void puterr( char *str ){
	write(2, str, strlen(str));
}

void putstr( char *str ){
	write(1, str, strlen(str));
}

int isnum( char c ){
	if( c>='0' && c<='9' ) return 1;
	return 0;
}

int tonum( char c ){
	return c-'0';
}

void out52( char c ){
	write(1, "\x1b", 1 );
	write(1,&c, 1 );
}

void out52n( char c, char n ){
	out52( c );
	write(1,&n, 1 );
}

void out52nn( char c, char n, char m){
	out52( c );
	write(1,&n,1);
	write(1,&m,1);
}


char cconv( char c ){
	return colors[c];
}


/* check for matching fore/background colors w/ intensity bit
   we can't do intensity - so text disappears */
void ckint(){
	if( (fc ==  bc)  &&  inten ){
		fc = (fc + 1) & 0x7 ;
		out52n( 'b', fc );
	}
}

/* Send a char to the console, processing for ANSI/TELNET chars */
void charout( unsigned char c ){
	static char mode=0;
	int i;

	switch( mode ){
		/* normal mode */
	case 0: 
		if( c == 255 ){
			mode=3;
			return;
		}
		if( c != ESC ) 	write( 1, &c ,1 );
		else{ 
			mode=1;
		}
		return;
		/* ESC detected */
	case 1:
		if( c=='[' ){
			mode=2;
			return;
		}
		return;
		/* Multi-byte detected */
	case 2: 
		if( isnum(c) ){
			para[no]=para[no]*10+tonum(c);
			return;
		}
		if( c == ';' ){
			no++;
			return;
		}
		no++;
		goto final;
		/* Telnet IAC detected */
	case 3:
		switch( c ){
		case 255:
			write(1,&c,1);
			mode=0;
			return;

		case 251:
			mode=5;
			return;
		case 252:
			mode=6;
			return;
		case 253:
		case 254:
			if(hex) printf("<IAC><%x>",c);
			mode=4;
			return;
		}
		mode=0;
		return;
		/* send Telnet's WILLNOTs to server */
	case 4:
		if( hex )printf("<%x>\n", c );
		write( fddw, "\xff\xfc",2);
		write( fddw, &c, 1 );
		mode = 0;
		return;
		/* received a WILL */
	case 5:
		if( c == 1 ){
			lecho = 0;
			//write(fddw,"\xff\xfd",2);
			//write(fddw, &c, 1 );
			mode = 0;
			return;
		}
		write( fddw,"\xff\xfe",2);
		write( fddw, &c, 1);
		mode = 0;
		return;
		/* received a WONT */
	case 6:
		if( hex ) printf("opt<%x>\n", c );
		if( c == 1 ) lecho = -1;
		else{
			write( fddw, "\xff\xfe", 2 );
			write( fddw, &c, 1 );
		}
		mode = 0;
		return;

	}
	/* and the trailing command */
 final:
	//	printf("DEBUG: %d %d %d %c\n", para[0], para[1], para[2], c);
	switch( c ){
	case 'J':
		if( para[0] == 2 ){
			out52( 'H' );
			out52( 'J' );
			break;
		}
		if( para[1] == 0 ){
			out52( 'J' );
			break;
		}
	case 'H':
	case 'f':
		if( para[0]==0 && para[1]==0 ){
			out52( 'H' );
			break;
		}
		if( para[0]==0 ) para[0]=1;
		if( para[1]==0 ) para[1]=1;
		out52nn( 'Y', para[0]+' ', para[1]+' ');
		break;
	case 'K':
		if( para[0] == 0 ){
			out52( 'K');
			break;
		}
	case 'm':
		for( i=0; i<no; i++ ){
			if( para[i]>=30 && para[i]<=37 ){
				fc=cconv(para[i]-30);
				out52n( 'b', fc );
				ckint();
			}
			if( para[i]>=40 && para[i]<=47 ){
				bc=cconv(para[i]-40);
				out52n( 'c', bc );
				ckint();
			}
			if( para[i]==0 ){
				out52n( 'b', '4' );
				out52n( 'c', '0' );
				inten=0;
			}
			if( para[i]==1 ){
				inten=1;
				ckint();
			}		
		}
		break;
	case 'C':
	case 'D':
	case 'A':
	case 'B':
		if( para[0]==0 ) para[0]=1;
		for( i=0; i<para[0]; i++) out52(c);
		break;
	case 'n':
		if( para[0]==6 ){
			write(fddw,"\x1b[1;1R",6);
			out52( 'H' );
			out52( 'J' );
		} 
		break;
		//	default:

	}
	para[0]=0;
	para[1]=0;
	para[2]=0;
	para[3]=0;
	no=0;
	mode = 0;
}

/* print string to console under ANSI/TELNET */
int mywrite( char *ptr, int len ){
	int i=len;
	while( i-- ) charout( (unsigned char) *ptr++ );
	return len;
}

void quit(){
	tcsetattr( fddw, TCSANOW, &prior );
	tcsetattr( 0, TCSANOW, &lprior );
	fcntl(0, F_SETFL, flags );
	close( fddw );
	exit(0);
}

/* Add string to input buffer */
void addstr(char *s){
	for( ; (ibuf[opos++] = *s); s++ )
		if( lecho ) write(1,s,1);
	opos--;
}

void printd( char *s ){
	while( *s ) write( 1, s++, 1);
	write( 1, "\n", 1 );
}

void printhelp( void ){
	printd( "l linemode toggle" );
	printd( "e echo toggle");
	printd( "q quit");
	printd( "h IAC TELNET hex debugging");
	printd( "n NL / CRNL");
	printd( "^A send ^A to remote");
}

/* Read from standard input, nonblocking */
int myread( void ){
	static int icount=0;
	static int ipos=0;
	static char kbuf[256];
	static int mode=0;
	static int cmode=0;
	int l;

	switch( mode ){
	case 0: // waiting for input buffer to fill middle 
		l=read( 0, kbuf, 256 );
		if( ! l ) return 0;
		if( l < 0 ){
			if( errno == EAGAIN ){
				return 0;
			}
			else {
				close(fddw);
				quit();
			}
		}
		ipos=0;
		icount=l;
		mode=1;
	case 1: // processing middle buffer to input buffer
		for( ; ipos<icount; ipos++ ){
			char c=kbuf[ipos];
			if( cmode ){
				switch( c ){
				case '1':
					addstr( "tcp connect " );
					break;
				case '2':
					addstr( "nostalgiamud.l-w.ca 4000");
					break;
				case '3':
					addstr( "aardmud.org 23" );
					break;
				case '4':
					addstr( "vert.synchro.net 23" );
					break;
				case '5':
					addstr( "madworld.bounceme.net 6400");
					break;
				case 'l':
					linemode = ~linemode ;
					break;
				case 'e':
					lecho = ~lecho;
					break;
				case 'q':
					quit();
				case 'h':
					hex= ~hex;
					break;
				case 'd':
					del= ~del;
					break;
				case 'n':
					crnl= ~crnl;
					break;
				case 1:
					ibuf[opos++]=1;
					break;
				case '?':
					printhelp();
					break;
				}
				cmode=0;
				continue;
			}
			if( c == 1 ){
				cmode=1;
				continue;
			}
			if( linemode ){
				if( c == 8 ){
					if( opos ){
						opos--;
						write(1,"\b \b",3);
						continue;
					}
					continue;
				}
				write(1,&c,1);
				if( c == '\n' ){
					int t=opos;
					opos=0;
					ibuf[t]=c;
					ipos++;
					return ++t;
				}
				ibuf[opos++]=c;
				continue;
			}
			if( lecho ) write(1,&c,1);
			if( (c == 8) && del ){
				ibuf[opos++]=127;
				continue;
			}
			if( (c== 0x0a) && crnl ){
				ibuf[opos++]=0x0d;
				ibuf[opos++]=0x0a;
				continue;
			}
			ibuf[opos++]=c;
			continue;
		}
		mode=0;
		if( ! linemode ){
			int t=opos;
			opos=0;
			return t;
		}
		return 0;
	}
	/* never reached */
	return 0;
}

void my_open( int argc, char *argv[]){
	if( !argv[optind] ){
		fddw=open("/dev/tty3", O_RDWR|O_NDELAY);
	}
	else{
		fddw=open(argv[optind], O_RDWR|O_NDELAY);
	}
	if( ! fddw ){
		puterr( "Cannot open device\n" );
		exit(-1);
	}
	
	tcgetattr( fddw, &prior );
	tcgetattr( fddw, &new );
	new.c_iflag &= ~ICRNL;
	new.c_lflag &= ~ECHO;
	new.c_lflag &= ~ICANON;
	new.c_oflag &= ~OPOST;
	tcsetattr( fddw, TCSANOW, &new );
}

int main( int argc, char *argv[]){
	int len;
	int opt;

	while( (opt=getopt( argc, argv, "p" ))>-1 ){
		switch( opt ){
		case 'p':
			pflag=1;
			break;
		}
	}


	tcgetattr( 0, &lprior );

	my_open( argc, argv );

	flags = fcntl(0, F_GETFL, 0);
	fcntl(0, F_SETFL, flags | O_NDELAY );


	tcgetattr( 0, &lnew );
	lnew.c_lflag &= ~ICANON;
	lnew.c_lflag &= ~ECHO;
	tcsetattr( 0, TCSANOW, &lnew );

	printf("Use Cntl-A ? for help.\n");

	while(1){
		char *pos=ibuf;
		len=myread();
		while( len > 0 ){
			int ret = write( fddw, pos, len );
			if( ret < 0 ){ 
				if( errno == EAGAIN ){
					ret=0;
				}
				else {
					close(fddw);
					if( pflag ){
						my_open( argc, argv );
						continue;
					}
					else quit();
				}
			}
			len=len-ret;
			pos+=ret;
		}
		len=read( fddw, ibuf, 127 );
		if( len < 0 ){
			if( errno != EAGAIN ){
				close(fddw);
				if( pflag ){
					my_open( argc, argv );
					continue;
				}
				else quit();
			}
		}
		pos = ibuf;
		while( len > 0){
			int ret = mywrite( pos, len );
			if( ret < 0 ){
				if( errno == EWOULDBLOCK ){
					ret=0;
				}
				else quit();
			}
			len=len-ret;
			pos+=ret;
		}
	}
}
