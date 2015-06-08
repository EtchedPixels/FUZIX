/* Header format of 16-bit
 * Seventh edition UNIX executables */

#ifndef _V7_A_OUT_H
#define _V7_A_OUT_H

#define V7_MAGIC4 0405	/* v7 overlay */
#define V7_OMAGIC 0407	/* I&D in one segment (impure) */
#define V7_NMAGIC 0410	/* read-only text */
#define V7_MAGIC3 0411	/* v7 separate I&D (pure) */
#define V7_ZMAGIC 0413	/* v8 demand load */

#define V7_HEADERLEN 16

struct  v7_exec {
    short magic;
    unsigned short textsize;
    unsigned short datasize;
    unsigned short bsssize;
    unsigned short symtabsize;
    unsigned short entry;
    unsigned short pad;
    unsigned short noreloc;
};

#endif /* _V7_A_OUT_H */
