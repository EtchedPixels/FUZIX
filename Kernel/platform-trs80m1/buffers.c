#include <kernel.h>
#include <kdata.h>

/*
 *	Must live in CODE2
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
 *	Slow crap implementation purely for testing. We need to go direct
 *	between bank 2 and user
 */
static uint8_t scratch2[512];
void blktou(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    __builtin_memcpy(scratch2, buf->__bf_data + off, len);
    uput(scratch2, uaddr, len);
}

void blkfromu(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    uget(uaddr, scratch2, len);
    __builtin_memcpy(buf->__bf_data + off, scratch2, len);
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

void bufsetup(void)
{
    uint8_t *p = bufdata;
    bufptr bp = bufpool;

    for(bp = bufpool; bp < bufpool_end; ++bp) {
        bp->__bf_data = p;
        p += BLKSIZE;
    }
}


/*
 *	Scratch buffers for syscall arguments - until we can rework
 *	execve and realloc to avoid this need
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
