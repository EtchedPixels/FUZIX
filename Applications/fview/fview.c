/*
    fview - a program to display windows .bmp files on fuzix.
    Copyright (C) 2016  Brett M. Gordon

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


   This will read the most common .bmp/dib formats of 1,2,4,8,16,24,or 32 bpp.
   It only translates to a 1 bpp screen format, with optional FS dithering.
   No scaling, offsetting, or colors.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/graphics.h>

struct bmp_header {
	char header[2];
	uint32_t size;
	uint16_t res1;
	uint16_t res2;
	uint32_t offset;
};

struct bmp_dib {
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t comp;
	uint32_t dsize;
	int32_t hres;
	int32_t vres;
	uint32_t colors;
};

struct bmp_palette {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t zero;
};

struct box {
	uint16_t size;
	uint16_t y;
	uint16_t x;
	uint16_t h;
	uint16_t w;
};

struct display entry_disp;  /* state of display on program entry */
struct display disp;        /* general struct for noodling with graphics ioctl */
uint8_t *lbuf;              /* bmp file line buffer */
uint8_t *obuf;              /* output line buffer to tty dev */
struct bmp_palette *pal;    /* table of palette entries */
int16_t *error1;            /* error buffer 1 in 1/16ths of error */
int16_t *error2;            /* error buffer 2 in 1/16ths of error */
int16_t *err_cur;           /* ptr to current line's errors */
int16_t *err_next;          /* ptr to next line's errors */
int dither=1;               /* should we dither? */
int pinfo=0;                /* just print info from bmp header and quit */
int rsize;                  /* bmp size of row in bytes */
int ssize;                  /* screen size of row in bytes */


uint16_t swizzle16( uint16_t d ){
	return d << 8 | d >> 8 ;
}


uint32_t swizzle32( uint32_t d ){
	return d >> 24 |
		( d >> 8 ) & 0xff00  |
		( d << 8 ) & 0xff0000  |
		( d << 24 );
}


/* Return true if big endian */
int big_endian(){
	uint16_t w = 0x1234;
	uint8_t *c = (uint8_t *)&w;
	if( *c == 0x12 ) return 1;
	return 0;
}

/* scan the tty driver's nodes.  Pick the one that gives us the most resolution at 1bpp, and can do a direct WRITE ioctl */
int scan_modes(void) {
  int i = 0;
  unsigned maxres=0;
  int ret=0;
  unsigned res;
  
  /* get state, info */
  if (ioctl(1, GFXIOC_GETINFO, &entry_disp) < 0)
	  return -1;
  if (ioctl(1, GFXIOC_GETINFO, &disp) < 0)
	  return -1;
  for(; 1; i++) {
	  disp.mode = i;
	  if (ioctl(1, GFXIOC_GETMODE, &disp) < 0)
		  break;
	  if( ! (disp.commands & GFX_WRITE) ) continue;
	  if( ! (disp.format & FMT_MONO_WB) ) continue;
	  res = disp.width * disp.height;
	  if( res > maxres ){
		  maxres = res;
		  ret=i;
	  }
  }
  return ret;
}


void exit_err_mess( char *mess ){
	fputs(mess, stderr);
	fputc( '\n', stderr );
	exit(1);
}

void exit_usage(){
	exit_err_mess("usage: fview -dp file");
}


/* This is an aproximation of a RGB -> intensity
   formula :) */
uint8_t intensity( struct bmp_palette *p ){
	uint16_t c;
	c = (p->red * 2);
	c += (p->green * 5);
	c += p->blue;
	return c/8;
}

/* This exchanges our error buffers - creating a 2 line-element ring buffer */
void flip_err( void ){
	if( err_cur == error1 ){
		err_cur = error2;
		err_next = error1;
	}
	else{
		err_cur = error1;
		err_next = error2;
	}
}

/* Floyd-Steinberg Dithering - distribute luminance error amongst forward neighbors */
void fs_dist( int16_t e, uint16_t i ){
	/* distribute error */
	if( dither ){
		err_cur[ i + 1 ] = e * 7 ;
		err_next[ i - 1 ] += e * 3 ;
		err_next[ i ] += e * 5 ;
		err_next[ i + 1 ] += e ;
	}
}	 

/* Convert a row of non-paletted modes to 1 bpp screen row */
void bpp32( int no ){
	int i,j,k,c,x,e;
	int erri=1;
	uint8_t *out = obuf+sizeof(struct box);
	uint8_t *in = lbuf;
	uint8_t pix[8];
	struct bmp_palette p;

	flip_err();

	memset( err_next, 0, (disp.width+2)*sizeof(int) );

	for( i=0; i<ssize; i++){
		c=0;
		for( j=0; j<8; j++ ){
			if( in < lbuf + rsize ){
				switch( no ){
				case 16:
					/* bit manip a 5-5-5-1 layout,
					   while scaling the result up to 
					   8bits from 5.
					*/
					p.green = (*(in+1) << 1) & 0xf8;
					p.red   = (*(in+1) << 6) | *in >> 5;
					p.blue  = *in << 3 ;
					in += 2;
					break;
					/* 32 is just 24 with a leading 0 */
				case 32:
					in++;
				case 24:
					p.blue = *in++;
					p.green = *in++;
					p.red = *in++;
				}
			}
			else{
				/* we're past src line end... make blank. */
				p.green = 0;
				p.red = 0 ;
				p.blue = 0 ;
			}
			c = c << 1;
			x= intensity( &p );
			if( dither )
				x += err_cur[ erri ]/16;
			if( x > 127 ){
				c++;
				e=x-256;
			}
			else{
				e=x;
			}
			fs_dist( e, erri );
			erri++;
		}
		*out++ = c; /* every 8 samples write c to dest */
	}
}


/* Convert 1 row of paletted mode to 1 bpp screen row */
void bpp( int no ){
	int i,j,k,c,x,e;
	int erri=1;
	uint8_t *out = obuf+sizeof(struct box);
	uint8_t *in = lbuf;
	uint8_t pix[8];

	flip_err();

	memset( err_next, 0, (disp.width+2)*sizeof(int) );

	for( i=0; i<ssize; i++){
		c=0;
		for( j=0; j<no; j++ ){

			if( in < lbuf + rsize ){
				switch( no ){
				case 1:
					pix[0]= *in >> 7;
					pix[1]= *in >> 6 & 1;
					pix[2]= *in >> 5 & 1;
					pix[3]= *in >> 4 & 1;
					pix[4]= *in >> 3 & 1;
					pix[5]= *in >> 2 & 1;
					pix[6]= *in >> 1 & 1;
					pix[7]= *in & 1;
					break;
				case 2:
					pix[0]= *in >> 6;
					pix[1]= *in >> 4 & 0x3;
					pix[2]= *in >> 2 & 0x3;
					pix[3]= *in & 0x3;
					break;
				case 4:
					pix[0]= *in >> 4;
					pix[1]= *in & 0xf;
					break;
				case 8:
					pix[0]= *in;
					break;
				}
				in++;
			}
			else{
				for( k=j; k<8/no; k++ ) pix[k]=0;
			}

			for( k=0; k<8/no; k++ ){
				c = c << 1;
				x= intensity( &(pal[ pix[k] ]) );

				if( dither )
					x += err_cur[ erri ]/16;
				if( x > 127 ){
					c++;
					e=x-256;
				}
				else{
					e=x;
				}
				fs_dist( e, erri );
				erri++;
			}
		}
		*out++ = c;
	}
	
}


int main( int argc, char *argv[] )
{
	int fd,ret;
	struct bmp_header h; /* bitmap file header */
	struct bmp_dib d;    /* bitmap dib header */
	struct box *lbox;    /* ptr to ioctl's len/box struct */
	int i,j;
	int llb;    /* last screen Y of bmp (if bmp height < screen height ) */

	if( argc<2 )
		exit_usage();

	for( i=1; i < argc; i++ ){
		if( argv[i][0] != '-' )
			break;
		for( j=1; j < strlen(argv[i]); j++){
			switch( argv[i][j] ){
			case 'd':
				dither=0;
				break;
			case 'p':
				pinfo=1;
				break;
			default:
				exit_usage();
			}
		}
	}

	/* 
	 *
	 * Open the bmp file and get it ready
	 *
	 *
	*/

	fd=open( argv[i], O_RDONLY );
	if( fd<0 )
		exit_err_mess("error opening bitmap");

	/* get file header */
	ret = read( fd, &h, sizeof(h) );
	if( ret < sizeof(h) )
		exit_err_mess("Error reading bmp file header");

	/* swizzle bitmap fields */
	if( big_endian() ){
		h.size = swizzle32( h.size );
		h.offset = swizzle32( h.offset );
	}

	/* get dib header */
	ret = read( fd, &d, sizeof(d) );
	if( ret < sizeof(d) )
		exit_err_mess("Error reading dib header");

	/* swizzle its fields */
	if( big_endian() ){
		d.size = swizzle32( d.size );
		d.width = swizzle32( d.width );
		d.height = swizzle32( d.height );
		d.planes = swizzle16( d.planes );
		d.bpp = swizzle16( d.bpp );
		d.comp = swizzle32( d.comp );
		d.dsize = swizzle32( d.dsize );
		/* -- skip swizzling some unused data -- */
		d.colors = swizzle32( d.colors );
	}

	/* get pallete table */
	if( lseek( fd, sizeof(h)+d.size, SEEK_SET ) < 0 )
		exit_err_mess("seek error in palette table");
	pal=malloc( sizeof(struct bmp_palette)*d.colors );
	if( pal == NULL )
		exit_err_mess("cannot malloc pallete buffer");
	ret= read( fd, pal, sizeof(struct bmp_palette)*d.colors);
	if( ret != sizeof(struct bmp_palette)*d.colors )
		exit_err_mess("read error loading palette");


	printf("header: %c%c\n", h.header[0], h.header[1] );
	printf("width: %d, height: %d\n", (uint16_t)d.width, (uint16_t)d.height );
	printf("bpp: %d\n", d.bpp );
	printf("pallete entries: %d\n", (uint16_t)d.colors );

	/* verify bitmap format */
	if( h.header[0]!='B' || h.header[1]!='M' )
		exit_err_mess("bad file header");
	if( d.size < 40 )
		exit_err_mess("unsupported bmp format");
	if( d.comp )
		exit_err_mess("compression not supported");

	if( pinfo ) exit(0);

	/* alloc bmp line buffers */
	rsize = (( d.bpp * d.width + 31 ) / 32 ) * 4;

	lbuf=malloc( rsize );
	if( lbuf == NULL )
		exit_err_mess("cannot malloc line buffer");


	if( lseek( fd, h.offset, SEEK_SET)<0 )
		exit_err_mess("seek error in bitmap file");

	/*
	 * 
	 *  Setup video mode / tty
	 *
	 */

	disp.mode=scan_modes();
	if( disp.mode < 0 )
		exit_err_mess("No suitable graphics mode found");

	if( ioctl( 1, GFXIOC_SETMODE, &disp ) < 0 )
		exit_err_mess("Cannot set graphics mode");

	ssize=disp.width / 8;

	obuf=malloc( ssize + sizeof(struct box) );
	if( obuf == NULL )
		exit_err_mess("cannot malloc dev buffer");

	lbox=(struct box *)obuf;
	lbox->size = ssize + sizeof(struct box);
	lbox->x = 0;
	lbox->h = 1;
	lbox->w = ssize;

	/* calc starting screen Y*/
	if( d.height < disp.height )
		llb = d.height-1;
	else llb = disp.height-1;

	/* Allocate dithering error buffers
	   we add an extra 2 samples here because dithering requires
	   a sample behind and in front of our subject pixel
	*/
	error1 = calloc( disp.width + 2, sizeof( int16_t ) );
	error2 = calloc( disp.width + 2, sizeof( int16_t ) );
	if( error1 == NULL | error2 == NULL )
		exit_err_mess("cannot alloc dither buffers");
	flip_err();


	/* main loop - for each row of pixels  
	   read a row from bmp file, tranlate to a row of screen data,
	   and send row data to tty
	 */
	for( i=disp.height-1; i>-1; i-- ){
		if( i <= llb ){
			/* get a line from the file */
			ret=read( fd, lbuf, rsize );
			if( ret < rsize ) {
				printf("file read error: %d\n", ret );
				goto leave;
			}

			/* pixate the transels here */
			switch( d.bpp ){
			case 1:
			case 2:
			case 4:
			case 8:
				bpp( d.bpp );
				break;
			case 16:
			case 24:
			case 32:
				bpp32( d.bpp );
				break;
			default:
				printf("unsupported bpp.\n");
				goto leave;
			}
		}
		else{
			memset( obuf+sizeof(struct box), 0, ssize );
		}
		/* output line to video device */
		lbox->y = i;
		ret=ioctl( 0, GFXIOC_WRITE, obuf );
		if( ret < 0 ){
			printf("dev write error: %d\n", ret );
			goto leave;
		}
	}

	getchar();
 leave:
	/* reset video mode */
	ioctl( 1, GFXIOC_SETMODE, &entry_disp );

	exit(0);
}       
