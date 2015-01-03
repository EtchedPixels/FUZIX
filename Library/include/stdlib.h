#ifndef __STDLIB_H
#define __STDLIB_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <malloc.h>
#include <syscalls.h>

/* Don't overwrite user definitions of NULL */
#ifndef NULL
#define NULL ((void *) 0)
#endif

/* Returned by `div' */
typedef struct {
	int	quot;		/* Quotient */
	int	rem;		/* Remainder */
} div_t;

/* Returned by `ldiv' */
typedef struct {
	long int quot;		/* Quotient */
	long int rem;		/* Remainder */
} ldiv_t;

/* For program termination */
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

extern void exit __P((int));
extern void abort __P((void));


extern int rand __P((void));
extern void srand __P((uint seed));

extern char *__longtoa __P((unsigned long, char *, int, char, char));
extern char *itoa __P((int value, char *strP, int radix));
extern char *ultoa __P((unsigned long value, char *strP, int radix));
extern char *ltoa __P((long value, char *strP, int radix));

extern int atoi __P((char *str));
extern long atol __P((char *strP));

extern char *_itoa __P((int value));
extern char *_ltoa __P((long value));
extern char *_ultoa __P((unsigned long value));

extern char *__ultostr __P((unsigned long value, int radix));
extern char *__ltostr __P((long value, int radix));

extern long strtol __P ((const char * nptr, char ** endptr, int base));
extern unsigned long strtoul __P ((const char * nptr,
				   char ** endptr, int base));
#ifndef __HAS_NO_DOUBLES__
extern double strtod __P ((const char * nptr, char ** endptr));
#endif

extern char **environ;
extern char *getenv __P((char *));
extern int putenv __P((char *));
extern int setenv __P((char *, char *, int));
extern void unsetenv __P((char *));

typedef void (*atexit_t) __P((int));
typedef void (*onexit_t) __P((int, void *));
extern int atexit __P((atexit_t));
extern int on_exit __P((onexit_t, void *arg));
extern onexit_t __cleanup;

extern char *crypt __P((char *__key, char *__salt));

typedef int (*cmp_func_t) __P((void *, void *));

extern int _bsearch;
extern void *bsearch __P((void *key, void *base, size_t num, size_t size, cmp_func_t cmp));
extern void *lfind __P((void *key, void *base, size_t *num, size_t size, cmp_func_t cmp));
extern void *lsearch __P((void *key, void *base, size_t *num, size_t size, cmp_func_t cmp));
extern void *_qbuf;
extern void qsort __P((void *base, size_t num, size_t size, cmp_func_t cmp));

extern int opterr;
extern int optind;
extern char *optarg;
extern int getopt __P((int argc, char *argv[], char *optstring));

extern char *getpass(char *prompt);

extern int _argc;
extern char **_argv;

#endif /* __STDLIB_H */
