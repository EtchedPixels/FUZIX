#ifndef __VARARGS_H
#define __VARARGS_H

#ifndef __STDARG_H
typedef void *va_list;

#define va_dcl va_list va_alist;
#define va_start(ap) (ap) = (va_list)&va_alist
#define va_arg(ap,t) ((t *)(((char *)(ap)) += sizeof(t)))[-1]
#define va_end(ap) (ap) = NULL

#endif
#endif

