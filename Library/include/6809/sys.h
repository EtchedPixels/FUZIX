#ifndef __SYS_H
#define __SYS_H

/* set userspace SWI vectors. ioctl(fd, request, char *data),
   where data is a pointer to a handler routine:
   void swi2_handler( void );
   if data is NULL, then the kernel will just do nothing and
   rti back to user.
 */
#define IOC_SET_SWI2  0710
#define IOC_SET_SWI3  0711

#endif
