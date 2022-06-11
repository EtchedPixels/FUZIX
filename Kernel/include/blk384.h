/*
 *	Rather less sane 384 byte block layout for 400 byte physical media
 */

#define BLKSIZE		384
#define INO_PER_BLOCK	6
/* We'll need to switch this to a little asm helper that knows about adding
   and right shifting (* 3 and div 1024) */
#define BLOCK(x)	(x / 384)

/* Help the compilers out by preventing any 32bit promotions. Will want an
   asm helper ? */
#define BLKOFF(x)	(((uint16_t)(x)) % 384)

#define BLK_TO_OFFSET(x)	((x) * 384)

#define SMOUNTED  12743   /* Magic number to specify mounted filesystem */
