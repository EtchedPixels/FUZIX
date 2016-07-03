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
	uint8_t zero;
	uint8_t blue;
	uint8_t green;
	uint8_t red;
};

struct box {
	uint16_t size;
	uint16_t y;
	uint16_t x;
	uint16_t h;
	uint16_t w;
};

struct display entry_disp;  /* state of display on program entry */
struct display disp;        /* general struct for noodling */
uint8_t *lbuf;              /* bmp line buffer */
uint8_t *obuf;              /* output line buffer to tty dev */
struct bmp_palette *pal;    /* table of palette entries */
int16_t *error1;            /* error buffer 1 */
int16_t *error2;            /* error buffer 2 */
int16_t *err_cur;           /* ptr to current line's errors */
int16_t *err_next;          /* ptr to next line's errors */

#ifndef LITTLE_ENDIAN
uint16_t swizzle16( uint16_t d ){
	return d << 8 | d >> 8 ;
}


uint32_t swizzle32( uint32_t d ){
	return d >> 24 |
		( d >> 8 ) & 0xff00  |
		( d << 8 ) & 0xff0000  |
		( d << 24 );
}
#endif

void println( uint8_t *buf){
	int i;
	for( i=0; i<42; i++){
		printf("%.2x ", buf[i] );
	}
	printf("\n");
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
	  if( disp.width != 256 ) continue;
	  res = disp.width * disp.height;
	  if( res > maxres ){
		  maxres = res;
		  ret=i;
	  }
  }
  return ret;
}

void exit_err_mess( char *mess ){
	fprintf( stderr, mess );
	fputc( '\n', stderr );
	exit(1);
}

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

void fs_dist( int16_t e, uint16_t i ){	
	/* distribute error */
	err_cur[ i + 1 ] = e * 7 ;
	err_next[ i - 1 ] += e * 3 ;
	err_next[ i ] += e * 5 ;
	err_next[ i + 1 ] += e ;
}	 

void bpp32( ){
	int i,j,k,c,x,e;
	int erri=1;
	uint8_t *out = obuf+sizeof(struct box);
	uint8_t *in = lbuf;
	uint8_t pix[8];
	struct bmp_palette p;

	flip_err();
	memset( err_next, 0, 516 );
	for( i=0; i<32; i++){
		c=0;
		for( j=0; j<8; j++ ){
			/* fixme: the following color translation only
			   works with big endian... add a ifdef for little */
			//in++;
			//p.blue = *in++;
			//p.green = *in++;
			//p.red = *in++;

			c = c << 1;
			x= intensity( (struct bmp_palette)in ) + err_cur[ erri ]/16;
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
		*out++ = c;
	}
	
}	


/* Tranlate a line worth of source data to 1 bpp */
void bpp( int no ){
	int i,j,k,c,x,e;
	int erri=1;
	uint8_t *out = obuf+sizeof(struct box);
	uint8_t *in = lbuf;
	uint8_t pix[8];

	flip_err();
	memset( err_next, 0, 516 );
	for( i=0; i<32; i++){
		c=0;
		for( j=0; j<no; j++ ){
			switch( no ){
			case 2:
				pix[0]= *in >> 6;
				pix[1]= *in >> 4 & 0x3;
				pix[2]= *in >> 2 & 0x3;
				pix[3]= *in++ & 0x3;
				break;
			case 4:
				pix[0]= *in >> 4;
				pix[1]= *in++ & 0xf;
				break;
			case 8:
				pix[0]= *in++;
				break;
				/* default case should be caught by caller */
			}
			for( k=0; k<8/no; k ++ ){
				c = c << 1;
				x= intensity( &(pal[ pix[k] ]) ) + err_cur[ erri ]/16;
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
	struct bmp_header h;
	struct bmp_dib d;
	int rsize;
	struct box *lbox;
	int i,j;
		

	if( argc<2 )
		exit_err_mess("usage: fview file");
	
	fd=open( argv[1], O_RDONLY );
	if( fd<0 )
		exit_err_mess("error opening bitmap");

	/* get file header */
	ret = read( fd, &h, sizeof(h) );
	/* swizzle bitmap fields */
#ifndef LITTLE_ENDIAN
	h.size = swizzle32( h.size );
	h.offset = swizzle32( h.offset );
#endif

	/* get dib header */
	ret = read( fd, &d, sizeof(d) );
	/* swizzle its fields */
#ifndef LITTLE_ENDIAN
	d.size = swizzle32( d.size );
	d.width = swizzle32( d.width );
	d.height = swizzle32( d.height );
	d.planes = swizzle16( d.planes );
	d.bpp = swizzle16( d.bpp );
	d.comp = swizzle32( d.comp );
	d.dsize = swizzle32( d.dsize );
	/* -- skip swizzling some unused data -- */
	d.colors = swizzle32( d.colors );
#endif

	/* get pallete table */
	if( lseek( fd, sizeof(h)+d.size, SEEK_SET ) < 0 )
		exit_err_mess("seek error in palette table");
	pal=malloc( sizeof(struct bmp_palette)*d.colors );
	if( pal == NULL )
		exit_err_mess("cannot malloc pallete buffer");
	ret= read( fd, pal, sizeof(struct bmp_palette)*d.colors);
	if( ret != sizeof(struct bmp_palette)*d.colors )
		exit_err_mess("read error loading palette");

	/* Allocate dithering error buffers */
	/* fixme: need to check for malloc errors */
	error1 = calloc( 256 + 2, sizeof( int16_t ) );
	error2 = calloc( 256 + 2, sizeof( int16_t ) );
	if( error1 == NULL | error2 == NULL )
		exit_err_mess("cannot alloc dither buffers");
	flip_err();
	

	printf("header: %c%c\n", h.header[0], h.header[1] );
	printf("width: %d, height: %d\n", (uint16_t)d.width, (uint16_t)d.height );
	printf("bpp: %d\n", d.bpp );
	printf("pallete entries: %d\n", (uint16_t)d.colors );


	rsize = (( d.bpp * d.width + 31 ) / 32 ) * 4;

	/* fixme: what if bmp width < screen width ? */
	lbuf=malloc( rsize );
	if( lbuf == NULL )
		exit_err_mess("cannot malloc line buffer");

	obuf=malloc( disp.width / 8 + sizeof(struct box) );
	if( obuf == NULL )
		exit_err_mess("cannot malloc dev buffer");

	lbox=(struct box *)obuf;


	if( lseek( fd, h.offset, SEEK_SET)<0 )
		exit_err_mess("seek error in bitmap file");

	lbox->size = 40;
	lbox->x = 0;
	lbox->h = 1;
	lbox->w = 32;

	/* set video mode */
       
	disp.mode=scan_modes();
	if( disp.mode < 0 )
		exit_err_mess("No suitable graphics mode found");

	if( ioctl( 1, GFXIOC_SETMODE, &disp ) < 0 )
		exit_err_mess("Cannot set graphics mode");


	/* loop */
	for( i=disp.height-1; i>-1; i-- ){
		/* get a line from the file */
		ret=read( fd, lbuf, rsize );
		if( ret < rsize ) {
			printf("file read error: %d\n", ret );
			goto leave;
		}
		/* pixate the transels here */
		switch( d.bpp ){
		case 1:
			memcpy( obuf+sizeof(struct box), lbuf, 32 );
			break;
		case 2:
		case 4:
		case 8:
			bpp( d.bpp );
			break;
		case 32:
			bpp32( );
			break;
		default:
			printf("unsupported bpp.\n");
			goto leave;
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
	/* unset video mode */
	ioctl( 1, GFXIOC_SETMODE, &entry_disp );

	exit(0);
}       
