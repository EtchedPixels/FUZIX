/*
 *	Rather less sane layout for 400 byte physical media
 *
 *	Inodes are packed 6 per block with the end of the block unused
 *	Directories are 10 per block with each entry internally padded
 *	Files use the full 400 bytes.
 */

#define BLKSIZE		400
#define INO_PER_BLOCK	6

#define BLOCK(x)	(x / 400)

/* Help the compilers out by preventing any 32bit promotions. Will want an
   asm helper ? */
#define BLKOFF(x)	(((uint16_t)(x)) % 400)

#define BLK_TO_OFFSET(x)	((x) * 400)

#define SMOUNTED  12743   /* Magic number to specify mounted filesystem */

/* Size of a directory. They can contain padding internally but a disk block
   must be divisible exactly into directory entries */

#define FILENAME_LEN	30
#define DIR_LEN		40	/* 8 bytes padding */
