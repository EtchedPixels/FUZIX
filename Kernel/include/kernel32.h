/*
 *	Functionality only used in the 32bit ports
 */

#ifndef __FUZIX__KERNEL_32_H
#define __FUZIX__KERNEL_32_H

/* malloc.c */
extern void *kmalloc(size_t);
extern void *kzalloc(size_t);
extern void kfree(void *);
extern void kmemaddblk(void *, size_t);
extern unsigned long kmemavail(void);
extern unsigned long kmemused(void);

/* flat.c */
extern void pagemap_switch(ptptr p, int death);
extern uaddr_t pagemap_base(void);
#define PROGLOAD pagemap_base()
extern uint32_t ugetl(void *uaddr, int *err);
extern int uputl(uint32_t val, void *uaddr);

extern void install_vdso(void);

#endif
