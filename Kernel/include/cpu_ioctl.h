#ifndef CPU_IOCTL_H
#define CPU_IOCTL_H

/* set userspace SWI vectors. ioctl(fd, request, char *data),
   where data is a pointer to a handler routine:
   void swi2_handler( void );
   if data is NULL, then the kernel will just do nothing and
   rti back to user.
*/
#define CPUIOC_6809SWI2 0710
#define CPUIOC_6809SWI3 0711

#endif
