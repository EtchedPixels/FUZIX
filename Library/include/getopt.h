#ifndef __GETOPT_H
#define __GETOPT_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern char *optarg;
extern int opterr;
extern int optind;

extern int getopt __P((int argc, char **argv, char *shortopts));

#endif /* __GETOPT_H */
