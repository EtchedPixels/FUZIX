/*
 *	Defines for the usual 512 byte file system layout
 */

#define BLKSIZE		512
#define INO_PER_BLOCK	8
#define BLKSHIFT	9
#define BLOCK(x)	((x) >> BLKSHIFT)
#define BLKMASK		511

/* Help the 8bit compilers out by preventing any 32bit promotions */
#define BLKOFF(x)	(((uint16_t)(x)) & BLKMASK)

#define BLK_TO_OFFSET(x)	((x) << BLKSHIFT)

#define SMOUNTED  12742   /* Magic number to specify mounted filesystem */
