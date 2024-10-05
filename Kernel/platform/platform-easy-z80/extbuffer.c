#include <kernel.h>
#include <kdata.h>
#include <printf.h>

extern uint8_t *bdest;
extern uint16_t blen;
extern void do_blkzero(uint8_t *ptr) __z88dk_fastcall;
extern void do_blkcopyk(uint8_t *src) __z88dk_fastcall;
extern void do_blkcopyul(uint8_t *src) __z88dk_fastcall;
extern void do_blkcopyuh(uint8_t *src) __z88dk_fastcall;

/*
 *	Must live in CODE2
 */

void blktok(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    bdest = kaddr;
    blen = len;
    do_blkcopyk(buf->__bf_data + off);
}

void blkfromk(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    bdest = buf->__bf_data + off;
    blen = len;
    do_blkcopyk(kaddr);
}

/* FIXME: work out a nice way to share the logic */
void blktou(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    uint16_t split;
    bdest = uaddr;
    blen = len;
    /* If it's all below 16K or all over 32K then use the 0x4000 window */
    if ((uint16_t)uaddr + len < 0x4000 || (uint16_t)uaddr > 0x8000) {
        do_blkcopyul(buf->__bf_data + off);
        return;
    }
    /* If it's all below 0x8000 then use the 0x8000 window */
    if ((uint16_t)uaddr + len < 0x8000) {
        do_blkcopyuh(buf->__bf_data + off + 0x4000);
        return;
    }
    /* Split case */
    split = 0x8000 - (uint16_t)uaddr;
    blen = split;
    do_blkcopyuh(buf->__bf_data + off + 0x4000);
    blen = len - split;
    bdest += split;
    do_blkcopyul(buf->__bf_data + off + split);
}

void blkfromu(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)
{
    uint16_t split;
    bdest = buf->__bf_data + off;
    blen = len;
    /* If it's all below 16K or all over 32K then use the 0x4000 window */
    if ((uint16_t)uaddr + len < 0x4000 || (uint16_t)uaddr > 0x8000) {
        do_blkcopyul(uaddr);
        return;
    }
    /* If it's all below 0x8000 then use the 0x8000 window */
    if ((uint16_t)uaddr + len < 0x8000) {
        bdest += 0x4000;
        do_blkcopyuh(uaddr);
        return;
    }
    /* Split case */
    split = 0x8000 - uaddr;
    blen = split;
    bdest += 0x4000;
    do_blkcopyuh(uaddr);
    blen = len - split;
    bdest += split- 0x4000;
    do_blkcopyul((uint8_t *)uaddr + split);
}

static uint8_t scratchbuf[64];

/* Worst case is needing to copy over about 64 bytes */
void *blkptr(struct blkbuf *buf, uint16_t offset, uint16_t len)
{
    if (len > 64)
        panic("blkptr");
    bdest = scratchbuf;
    blen = sizeof(scratchbuf);
    do_blkcopyk(buf->__bf_data + offset);
    return scratchbuf;
}

void blkzero(struct blkbuf *buf)
{
    do_blkzero(buf->__bf_data);
}

/*
 *	Scratch buffers for syscall arguments - until we can rework
 *	execve and realloc to avoid this need
 *
 *	The buffers must be valid in both kernel and buffer mappings.
 */

extern uint8_t workbuf[2][BLKSIZE];
static uint8_t tfree = 3;

void tmpfree(void *p)
{
  if (p == workbuf[0]) {
      tfree |= 1;
      return;
  }
  if (p == workbuf[1]) {
      tfree |= 2;
      return;
  }
  panic("tmpfree");
}

void *tmpbuf(void)
{
   if (tfree & 1) {
       tfree &= ~1;
       return workbuf[0];
   }
   if (tfree & 2) {
       tfree &= ~2;
       return workbuf[1];
   }
   panic("tmpbuf");
}
