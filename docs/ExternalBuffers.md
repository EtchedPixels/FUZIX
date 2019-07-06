# Buffer Placement And Bounce Buffer Considerations

## External Buffers

If your platform requires you to place the buffer cache outside of the kernel
address space (or you want to for other reasons) then you need to provide the
following functionality and define CONFIG_BLKBUF_EXTERNAL.

    void blktok(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)

    Copy the section of the buffer from offset off, for len bytes into the kernel
    addres space beginning at kaddr.

    void blkfromk(void *kaddr, struct blkbuf *buf, uint16_t off, uint16_t len)

    Copy from kaddr into the section of the buffer from offset off, for len bytes.

    void blktou(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)

    Copy the section of the buffer from offset off, for len bytes into the current
    user address space starting at uaddr.

    void blkfromu(void *uaddr, struct blkbuf *buf, uint16_t off, uint16_t len)

    Copy from uaddr in the current user address space into the section of the
    buffer from offset off, for len bytes.

    void blkzero(struct blkbuf *buf)

    Fill the given buffer data with zero bytes

    void *blkptr(struct blkbuf *buf, uint16_t offset, uint16_t len)

    Return a pointer to a temporary copy of the data in buf from offset to offset
    + len. The copy is lost on further calls. The largest value of len required is
    currently about 64 bytes.

The buffers have a private __bf_data field which is native platform sized
pointer. This is entirely free for the user of your buffer cache manager
and should be filled in at initialization time using whatever scheme you
prefer (eg address within the block buffer bank).

## Aligned Buffers

Some platforms require that disk transfer buffers are aligned or do not
cross certain boundaries. For direct I/O it is not possible to guarantee
these but for cache buffers it is. This can also be useful as you can then
also keep a private cache buffer from the same aligned block to bounce awkwardly
aligned I/O to user space.

To control buffer alignment implement external buffers as described in
the previous chapter. If however your platform does not need, or want the
buffers external to the kernel address space then you can also define
CONFIG_BLKBUF_HELPERS and the kernel will instead use the normal buffer
access routines. In that case __bf_data becomes a kernel pointer.

## MMIO Overlap

Certain platforms (notably MSX systems) have both the memory and disk
devices mapped into the same address space. This causes problems when the
buffer to be written is in the same address range as the I/O device. This
can be handled in two ways

If the hardware permits it then the kernel driver can map the overlapping
block at a different location. In MSX2 for example this is possible. Great
care must be taken with requests that cross blocks.

If the hardware does not then transfers in this range will need to use a
bounce buffer. Because the disk cache is non-recursive and to avoid the
possibility of deadlocks any bounce buffer must not be part of the disk
cache or must be allocated and pinned at boot if it is. The same bounce
buffer can be shared by disk devices.

## Large Block Sizes

When the platform had a disk device with a block size of over 512 bytes it
will need to implement deblocking. Fortunately such devices are rare,
although sometimes found in very old disk controllers. When deblocking it is
worth spotting and optimizing the case where a request is made for multiple
sequential blocks. That particular pattern is common in program
swapping and larger raw disk I/O and will give significant speedups over
deblocking when swapping.

## Heads And Tails

A tiny number of early disk controllers perform a single linear DMA and
require some header and tail data is present, and in some cases also writes
it. These devices can be managed in the disk cache by spacing buffers with
room for the head and tail data. Because only a single transaction will be
outstanding it is also possible to overlap the tail and head (as if a block
head is being used the tail being used will not be the tail of the block
before). In all these cases the actual bf_data pointer remains pointing to
the start of the user requested data. 

If the device also requires deblocking then this can be handled when
deblocking.

For the same reason as with large block sizes it can be a significant speed
up when doing large transfers of user space pages (swap or direct to user
accesses) to actually save a few bytes before or after the existing user
data, over-write then, do the transfer and if needed put them back. In
particular a long linear read or write for swap will benefit from this. If
implementing this take care when at the bottom or top of the user address
space not to scribble into unavailable areas - or leave small amounts of
padding between the user and kernel.

