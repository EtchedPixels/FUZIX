#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Must live in CODE3. We share this with TRS80model 1 so we ought
 *	to extract it for Z80.
 */

void blktok(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    __builtin_memcpy(kaddr, buf->__bf_data + off, len);
}

void blkfromk(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    __builtin_memcpy(buf->__bf_data + off, kaddr, len);
}

/*
 *	This works because our uput and uget (see trs80-bank.s) switch
 *	to kernel logical bank 2 when copying, as kernel bank 1 is only
 *	code so it knows that any copy must be to common or bank 2. Without
 *	that this would need a double buffer.
 */
void blktou(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    _uput(buf->__bf_data + off, uaddr, len);
}

void blkfromu(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    _uget(uaddr, buf->__bf_data + off , len);
}

static uint8_t scratchbuf[64];

/* Worst case is needing to copy over about 64 bytes */
void *blkptr(struct blkbuf *buf, uint16_t offset, uint16_t len)
{
    if (len > 64)
        panic("blkptr");
    __builtin_memcpy(scratchbuf, buf->__bf_data + offset, len);
    return scratchbuf;
}

void blkzero(struct blkbuf *buf)
{
    __builtin_memset(buf->__bf_data, 0, BLKSIZE);
}

extern uint8_t bufdata[];

/* This is called at start up to assign data to the first buffers, and then
   again to assign data to the extra allocated buffers */

static bufptr bnext = bufpool;
static uint8_t *bdnext = bufdata;

void bufsetup(void)
{
    bufptr bp;

    for(bp = bnext; bp < bufpool_end; ++bp) {
        bp->__bf_data = bdnext;
        bdnext += BLKSIZE;
    }
    bnext = bp;
}

/*
 *	Scratch buffers for syscall arguments - until we can rework
 *	execve and realloc to avoid this need. We can in theory put
 *	the second tmpbuf into bank3 private space
 */

static uint8_t tmp[2][BLKSIZE];
static uint8_t tfree = 3;

void tmpfree(void *p)
{
  if (p == tmp[0]) {
      tfree |= 1;
      return;
  }
  if (p == tmp[1]) {
      tfree |= 2;
      return;
  }
  panic("tmpfree");
}

void *tmpbuf(void)
{
   if (tfree & 1) {
       tfree &= ~1;
       return tmp[0];
   }
   if (tfree & 2) {
       tfree &= ~2;
       return tmp[1];
   }
   panic("tmpbuf");
}
