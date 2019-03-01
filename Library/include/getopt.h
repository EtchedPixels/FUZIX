#ifndef __GETOPT_H
#define __GETOPT_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern char *optarg;
extern int opterr;
extern int optind;
extern int optopt;

extern int getopt(int __argc, char * const __argv[], const char *__shortopts);

#endif /* __GETOPT_H */
