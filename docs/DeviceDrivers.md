# Device Drivers

## Introduction

Fuzix divides devices up the same way as classical Unix does. Block devices
provide block oriented disk interfaces and can be used to hold file systems.
Character devices provide an unstructured bytestream access. Unlike Unix all
block devices are implicitly also character devices with the character
interface providing raw disk access.

Certain subclasses of devices have a generic interface to the device layer
that provides its own interface to the platform specific code. Some generic
devices are also provided entirely by the OS core.

## Device Methods

Each Fuzix device has five methods. These do not all need to be implemented
and generic versions of them exist for default cases.

### open

This method is called when the device is accessed by a program opening it
via system call, or when the kernel opens it for internal usage. 

```
int device_open(uint_fast8_t minor, uint16_t flags)
````

minor is the minor number of the device node being opened and ranges from 0
to 255. flags is the full set of flags passed to open().

A typical open method checking the minor number is in range might look
something like this

````
    int device_open(uint_fast8_t minor, uint16_t flags)
    {
        if (minor > 0) {
            udata.u_error = ENODEV;
            return -1;
        }
        device_setup();
        return 0;
    }
````

Open should set u_error correctly if an error occurs and return -1. If it
succeeds then it should return 0

### close
````
    int device_close(uint_fast8_t minor)
````
This method is called when the device is closed. If an open file handle is
duplicated then it will be called on the final close of the duplicated
handle. Thus close() calls match the number of successful open calls.

Close can return -1 and an error code in u_error, or 0. Note that most
application code in the Unix world is incorrect and does not check if close()
fails so erroring on close is discouraged in the interests of compatibility.

### read and write
````
    int read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
    int write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
````
minor indicates the minor number from 0-255 of the device being accessed.
rawflag indicates the mode of access (see below), and flag holds the
persistent subset of flags passed on open. Notably this includes O_NDELAY
indicating that an error should be returned if the I/O cannot be completed
reasonably quickly.

The read method is called in three differing ways. For character devices
the raw flag will always be 1 (character I/O). For block devices it may be
0 (Block I/O) or 2 (Swap).


#### Character I/O

When the method is called for character I/O the following are set up

* udata.u_count: the number of bytes to transfer
* udata.u_base: the user space address to begin reading/writing
* udata.u_done: zero
* udata.u_offset: seek offset - usually meaningless

The method should transfer up to udata.u_count bytes of data. In the event
of an error the return value should be the number of bytes successfully
transferred and the error code should be saved for the next call. If no data
has been transferred then -1 should be returned and udata.u_error set
correctly. A return value of 0 is used to indicate an end of file on read,
meaning that some kind of end mark or event occurred. A return value of 0 on
write indicates that no more can be written.

In some cases such as disk I/O the request may need to be block aligned and
properly sized.

#### Block I/O

When this method is called for block I/O the following are set up

* udata.u_buf: the disk buffer for this request
* udata.u_block: the block number
* udata.u_blkoff: the offset within the block
* udata.u_nblock: the number of blocks
* udata.u_dptr: the kernel space address to begin reading/writing

The return values are the same as for character I/O, indicating the number
of bytes transferred.

#### Swap I/O

Swap I/O is a special case because of the memory management requirements. In
the swap case the values are set up as for block I/O except

* udata.u_buf: undefined
* udata.u_dptr: the swap mapped address to begin reading/writing
* swappage: the memory management page identifier for this transfer

Swap I/O in many cases is much like normal block I/O with a different
target, but on some platforms also involves higher level remapping occurring
above (and outside the view of) the block driver.

#### Helpers

Block devices also provide a character interface. To avoid duplication they
can call d_blkoff() in order to convert the character request into a block
like one but for user memory banks.

````
    if (rawflag == 1) {
            if (d_blkoff(shift))
                    return -1;
    }
````

#### Low Level Data Transfers

These are normally in assembler on banked machines as they need to be in
common space. On flat address space machines such as Motorola 68000 the
distinction becomes meaningless.

There are three assembler helpers that are used for bank switching to the
required bank.

* map_buffers - map the disk buffers into memory
* map_process_always - map the current process into memory
* map_for_swap - map the swap target into memory

and one for switching back, which must be done before returning from the
common code.

* map_kernel

As the kernel stack is in common space there is no need to stack switch.
Each map to a buffers, process or swap must be matched with exactly one
map_kernel, and they do not stack.

Certain memory models require non-standard device bank management. These are
described in the memory mapping descriptions for these memory types. Notably
this is needed for thunked Z80 systems with no common RAM.

### ioctl

The ioctl interface provides a generic interface for out of band control or
requests that do not fit a read/write mechanism. Some ioctls are generic
whilst others may be defined by a particular device or class of device.

````
    int ioctl(uint_fast8_t minor, uarg_t request, char *data);
````

The minor is the minor number of the device in question. request holds the
request code passed by the user program and data a pointer to the user
address passed by the program.

The ioctl interface returns -1 and sets udata.u_error on an error, or just
returns -1 for unknown requests.

