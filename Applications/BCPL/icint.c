/* Copyright (c) 2004 Robert Nordier.  All rights reserved. */
/* Changes to implement disk-based virtual/paged program store
   (c) 2016 Neal Crook.

   This is an interpreter for BCPL INTCODE: it implements the
   INTCODE virtual machine.

   The address space of the virtual machine is set to VSIZE
   (maximum value is 64*1024 locations, each of 16 bits -- these
   restrictions imposed by INTCODE).
   The address space is represented on the interpreter by a
   buffer of size PSIZE.

   By default, VSIZE is set to the value of PSIZE. If PAGEDMEM
   is defined at compile-time, VSIZE is set to maximum (64*1024)
   and a file on disk is used as a page file to handle the
   mismatch betwen VSIZE and PSIZE.

   The page file is controlled through a demand-paged algorithm
   implemented in pagein().

   If PAGEDMEM is not defined and PSIZE is too small, there will
   be inadequate address space to run the BCPL compiler or to
   recompile BCPL from source. This should result in a polite
   error like this:

   FAULT C=10495 P=18598 VSIZE=18432

   However, if the memory shortfall is too great, the result will
   be a segmentation fault rather than a polite error. The minimum
   size required to recompile BCPL from source is around 20*1024
   locations. This minimum is imposed on Linux hosts but not
   imposed in general because most FUZIX targets do not have
   enough memory available - exactly the reason why the PAGEDMEM
   functionality was implemented.
*/

/* $Id: icint.c,v 1.6 2004/12/11 12:01:53 rn Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "blib.h"

#define BLKSIZE 1024
#ifdef __linux__
/* this value is set to be large enough to re-compile BCPL on the host system */
#define PBLKS 20
#else
/* this value is set to be large enough to be buildable for "most" FUZIX targets. Make it smaller
   if your target is particularly short of user-space memory. */
#define PBLKS 15
#endif


#ifdef PAGEDMEM
#define VBLKS 63
#else
#define VBLKS PBLKS
#endif

#define PSIZE (BLKSIZE*PBLKS)
#define VSIZE (BLKSIZE*VBLKS)
#define MGLOB 1
#define MPROG 402

#define FALSE 0
#define TRUE 1

#define FSHIFT 13
#define IBIT 010000
#define PBIT 04000
#define GBIT 02000
#define DBIT 01000
#define ABITS 0777
#define WORDSIZE 16
#define BYTESIZE 8

#define LIG1 0012001
#define K2 0140002
#define X22 0160026

/* debug hook for VM tracing */
#if 1
#define itrace(x)
#define itrace2(x,y)
#define itrace3(x,y,z)
#define itrace4(x,y,z, a)
#else
#define itrace(x)		fprintf(stderr, x)
#define itrace2(x,y)		fprintf(stderr, x, y)
#define itrace3(x,y,z)		fprintf(stderr, x, y, z)
#define itrace4(x,y,z,a)	fprintf(stderr, x, y, z, a)
#endif


uint16_t M[PSIZE];
int fp;

static uint16_t G;
static uint16_t P;
static int Ch;
static int16_t *Labv;
static int Cp;
static uint16_t A;
static uint16_t B;
static uint16_t C;
static uint16_t D;
static uint16_t W;

static void assemble(void);
static void stw(uint16_t);
static void stc(uint16_t);
static void rch(void);
static int16_t rdn(void);
static void setlab(int);
static void labref(int16_t, uint16_t);
static int16_t interpret(void);
static int16_t icgetbyte(uint16_t, uint16_t);
static void icputbyte(uint16_t, uint16_t, uint16_t);

/* Abstract away the difference between the paged and non-paged implementations */
#ifdef PAGEDMEM

/* -1 if unmapped, n if this virtual page is in physical buffer n */
int vmap[VBLKS];
/* -1 if unmapped, n if physcal buffer n holds this virtual page */
int pmap[VBLKS];
/* stats for replacement algorithm: translation count - like a time-stamp */
int tcnt[VBLKS];
/* state of page */
#define PAGNEVER 0
#define PAGCLEAN 1
#define PAGDIRTY 2
int pagstate[VBLKS];

int translations = 0;
int swaps = 0;
int pfp;

/* Given a virtual page number that is not mapped, map it. Take care of
   any disk read/write.
*/
static void
pagein(uint16_t page)
{
    int i;
    int victim;

#if 0
    swaps++;
    fprintf(stderr, "Pagein: page=%d, swaps=%d, translations=%d\n",page, swaps, translations);
#endif

    /* First attempt: search pmap for an unassigned block */
    for (i=0; i<PBLKS; i++) {
        if (pmap[i] == -1) {
            /* success - assign physical block i to virtual page */
            pmap[i] = page;
            vmap[page] = i;
            pagstate[page] = PAGCLEAN;
            return;
        }
    }

    /* subsequent attempts assume ALL BLOCKS ARE ALLOCATED */

    /* Second attempt: search pmap for a clean block */
    for (i=0; i<PBLKS; i++) {
        if (pagstate[pmap[i]] == PAGCLEAN) {
            /* success - discard clean page from block, reassign block to virtual page */
            vmap[pmap[i]] = -1;
            pmap[i] = page;
            vmap[page] = i;
            if (pagstate[page] == PAGCLEAN) {
                /* page has been used before so need to pull it from disk.
                   Overwrite block i which is clean, so no need to save it.
                   Page on disk is at byte offset page*2*BLKSIZE
                   Page in physical buffer is at byte offset i*2*BLKSIZE
                   Going to transfer 2*BLKSIZE bytes
                */
                if ( (lseek(pfp, (uint16_t)(page*2*BLKSIZE), SEEK_SET) < 0) ||
                     (2*BLKSIZE != read(pfp, (unsigned char *)(M+i*BLKSIZE), 2*BLKSIZE)) ) {
                    perror("replace clean");
                    exit(1);
                }
            }
            return;
        }
    }

    /* Third attempt: search pmap for least-recently-used buffer to reassign */
    victim = 0;
    for (i=1; i<PBLKS; i++) {
        if (tcnt[pmap[i]] < tcnt[pmap[victim]]) {
            /* buffer i was last-used longer ago than the current victim
               .. this is crude because it ignores wrap-around, but there is
               no Correctness issue with a bad choice of victim
            */
            victim = i;
        }
    }

    /* victim is dirty so must be written to disk. Its offset in the
       pagefile is pmap[victim]*2*BLKSIZE. It comes from the physical
       buffer at byte offset victim*2*BLKSIZE. It is 2*BLKSIZE bytes.
    */
    if ( (lseek(pfp, (uint16_t)(pmap[victim]*2*BLKSIZE), SEEK_SET) < 0) ||
         (2*BLKSIZE != write(pfp, (unsigned char *)(M+victim*BLKSIZE), 2*BLKSIZE)) ) {
        perror("page-out dirty");
        exit(1);
    }
    pagstate[pmap[victim]] = PAGCLEAN;
    vmap[pmap[victim]] = -1;

    pmap[victim] = page;
    vmap[page] = victim;
    if (pagstate[page] == PAGCLEAN) {
        /* Page has been used before, and therefore must be read from disk.
           Page on disk is at byte offset page*2*BLKSIZE
           and it will go into the buffer at (of course) the same place as
           victim was written out from. Transfer 2*BLKSIZE bytes.
        */
        if ( (lseek(pfp, (uint16_t)(page*2*BLKSIZE), SEEK_SET) < 0) ||
             (2*BLKSIZE != read(pfp, (unsigned char *)(M+victim*BLKSIZE), 2*BLKSIZE)) ) {
            perror("replace dirty");
            exit(1);
        }
    }
}


void
wrM(uint16_t addr, uint16_t data)
{
    if (vmap[addr>>10] == -1) {
        pagein(addr>>10);
    }

    translations++;
    pagstate[addr>>10] = PAGDIRTY;
    tcnt[addr>>10] = translations;
    M[(vmap[addr>>10]<<10) | (addr & 0x3ff)] = data; /* hardwired for BLKSIZE=1024 */
}

void
add2M(uint16_t addr, uint16_t data)
{
    if (vmap[addr>>10] == -1) {
        pagein(addr>>10);
    }

    translations += 2;
    tcnt[addr>>10] = translations;
    M[(vmap[addr>>10]<<10) | (addr & 0x3ff)] = M[(vmap[addr>>10]<<10) | (addr & 0x3ff)] + data; /* hardwired for BLKSIZE=1024 */
}

uint16_t
rdM(uint16_t addr)
{
    if (vmap[addr>>10] == -1) {
        pagein(addr>>10);
    }

    translations++;
    tcnt[addr>>10] = translations;
    return M[(vmap[addr>>10]<<10) | (addr & 0x3ff)]; /* hardwired for BLKSIZE=1024 */
}

#else

#define wrM(address, data) M[address]=data
#define add2M(address, data) M[address]+=data
#define rdM(address) M[address]
#endif



static void writes(const char *p) {
 write(1, p, strlen(p));
}

#ifdef __linux__
static const char *_itoa(int v)
{
 static char buf[16];
 sprintf(buf, "%d", v);
 return buf;
}
#endif

static void
assemble(void)
{
    int16_t v[501];
    uint16_t f = 0;
    uint16_t i;

    Labv = v;
clear:
    for (i = 0; i <= 500; i++) Labv[i] = 0;
    Cp = 0;
next:
    rch();
sw:
    switch (Ch) {

    default: if (Ch == EOF) return;
        printf("\nBAD CH %c AT P = %d\n", Ch, P);
        goto next;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        setlab(rdn());
        Cp = 0;
        goto sw;

    case '$': case ' ': case '\n': goto next;

    case 'L': f = 0; break;
    case 'S': f = 1; break;
    case 'A': f = 2; break;
    case 'J': f = 3; break;
    case 'T': f = 4; break;
    case 'F': f = 5; break;
    case 'K': f = 6; break;
    case 'X': f = 7; break;

    case 'C': rch(); stc(rdn()); goto sw;

    case 'D': rch();
        if (Ch == 'L') {
            rch();
            stw(0);
            labref(rdn(), P - 1);
        } else
            stw(rdn());
        goto sw;

    case 'G': rch();
        A = rdn() + G;
        if (Ch == 'L') rch();
        else printf("\nBAD CODE AT P = %d\n", P);
        wrM(A, 0);
        labref(rdn(), A);
        goto sw;
    case 'Z': for (i = 0; i <= 500; i++)
        if (Labv[i] > 0) printf("L%d UNSET\n", i);
        goto clear;
    }
    W = f << FSHIFT;
    rch();
    if (Ch == 'I') { W = W + IBIT; rch(); }
    if (Ch == 'P') { W = W + PBIT; rch(); }
    if (Ch == 'G') { W = W + GBIT; rch(); }

    if (Ch == 'L') {
        rch();
        stw(W + DBIT);
        stw(0);
        labref(rdn(), P - 1);
    } else {
        uint16_t a = rdn();
        if ((a & ABITS) == a)
            stw(W + a);
        else { stw(W + DBIT); stw(a); }
    }
    goto sw;
}

static void
stw(uint16_t w)
{
    wrM(P++, w);
    Cp = 0;
}

static void
stc(uint16_t c)
{
    if (Cp == 0) { stw(0); Cp = WORDSIZE; }
    Cp -= BYTESIZE;
    add2M(P - 1, c << Cp);
}

static char buf[128];
static char *rcp = buf;
static char *rce = buf;

static int16_t rchget(void)
{
 rcp = buf;
 rce = buf + read(fp, buf, 128);
 if (rce <= rcp)
  return -1;
 return 0;
}

static void
rch(void)
{
    /* FIXME: blows up on EOF */
    for (;;) {
        if (rcp == rce)
          if (rchget()) {
           Ch = -1;
           return;
          }
        Ch = *rcp++;
        if (Ch != '/') return;
        do {
         if (rcp == rce)
          if (rchget()) {
            Ch = -1;
            return;
          }
          Ch = *rcp++;
       } while (Ch != '\n');
    }
}

static int16_t
rdn(void)
{
    int a = 0, b = FALSE;
    if (Ch == '-') { b = TRUE; rch(); }
    while ('0' <= Ch && Ch <= '9') { a = 10 * a + Ch - '0'; rch(); }
    if (b) a = -a;
    return a;
}

static void
setlab(int n)
{
    int16_t k = Labv[n];
    if (k < 0) printf("L%d ALREADY SET TO %d AT P = %d\n", n, -k, P);
    while (k > 0) {
	uint16_t kp = k;
        uint16_t nv = rdM(kp);
        /* Removing this debug check breaks under SDCC - FIXME check if
           compiler bug ! */
        if (n == 9499)fprintf(stderr, "setlab %d to %d\n", (unsigned int)kp, (unsigned int)P);
        wrM(kp, P);
        k = nv;
    }
    Labv[n] = -P;
}

static void
labref(int16_t n, uint16_t a)
{
    int16_t k = Labv[n];
//    if (n == 499)
//     fprintf(stderr, "Labref %d = %d\n", n, (int)k);
    if (k < 0) k = -k; else Labv[n] = a;
//    if (n == 499)
//    fprintf(stderr, "Mod %d by %d from %d\n", (unsigned int)a, (int)k, (unsigned int)M[a]);
    add2M(a, k);
}

static int16_t interpret(void)
{
fetch:
    itrace2("%d = ", C);

    if (C >= VSIZE || P >= VSIZE || C == 0) {
        fprintf(stderr, "FAULT C=%d P=%d VSIZE=%d\n", C, P, VSIZE);
        exit(1);
    }
    W = rdM(C++);
    if ((W & DBIT) == 0)
        D = W & ABITS;
    else
        D = rdM(C++);

    itrace3("OP %d, D %d ", W >> FSHIFT, D);

    if ((W & PBIT) != 0) { D += P; itrace("+P"); }
    if ((W & GBIT) != 0) { D += G; itrace("+G"); }
    if ((W & IBIT) != 0) { D = rdM(D); itrace("[]"); }

    itrace4("->%d [A%dB%d]\n", D, A, B);

    switch (W >> FSHIFT) {
    error: default: writes("\nINTCODE ERROR AT C = ");
                    writes(_itoa(C-1));
                    writes("\n");
        return -1;
    case 0: B = A; A = D; goto fetch;
    case 1: wrM(D, A); goto fetch;
    case 2: A = A + D; goto fetch;
    case 3: C = D; goto fetch;
    case 4: A = !A;
    case 5: if (!A) C = D; goto fetch;
    case 6: D += P;
        wrM(D, P); wrM(D + 1, C);
        P = D; C = A;
        goto fetch;
    case 7: switch (D) {
        default: goto error;
        case 1: A = rdM(A); goto fetch;
        case 2: A = -A; goto fetch;
        case 3: A = ~A; goto fetch;
        case 4: C = rdM(P + 1);
            P = rdM(P);
            goto fetch;
        case 5: A = (int16_t)B * (int16_t)A; goto fetch;
        case 6: A = (int16_t)B / (int16_t)A; goto fetch;
        case 7: A = (int16_t)B % (int16_t)A; goto fetch;
        case 8: A = (int16_t)B + (int16_t)A; goto fetch;
        case 9: A = (int16_t)B - (int16_t)A; goto fetch;
        case 10: A = B == A ? ~0 : 0; goto fetch;
        case 11: A = B != A ? ~0 : 0; goto fetch;
        case 12: A = (int16_t)B < (int16_t)A  ? ~0 : 0; goto fetch;
        case 13: A = (int16_t)B >= (int16_t)A ? ~0 : 0; goto fetch;
        case 14: A = (int16_t)B > (int16_t)A ? ~0 : 0; goto fetch;
        case 15: A = (int16_t)B <= (int16_t)A ? ~0 : 0; goto fetch;
        case 16: A = B << A; goto fetch;
        case 17: A = B >> A; goto fetch;
        case 18: A = B & A; goto fetch;
        case 19: A = B | A; goto fetch;
        case 20: A = B ^ A; goto fetch;
        case 21: A = B ^ ~A; goto fetch;
        case 22: return 0;
        case 23: B = rdM(C); D = rdM(C + 1);
            while (B != 0) {
                B--; C += 2;
                if (A == rdM(C)) { D = rdM(C + 1); break; }
            }
            C = D;
            goto fetch;

        case 24: selectinput(A); goto fetch;
        case 25: selectoutput(A); goto fetch;
        case 26: A = rdch(); goto fetch;
        case 27: wrch(A); goto fetch;
        case 28: A = findinput(A); goto fetch;
        case 29: A = findoutput(A); goto fetch;
        case 30: return A;
        case 31: A = rdM(P); goto fetch;
        case 32: P = A; C = B; goto fetch;
        case 33: endread(); goto fetch;
        case 34: endwrite(); goto fetch;
        case 35: D = P + B + 1;
                 wrM(D,     rdM(P));
                 wrM(D + 1, rdM(P + 1));
                 wrM(D + 2, P);
                 wrM(D + 3, B);
                 P = D;
                 C = A;
                 goto fetch;
        case 36: A = getbyte(A, B); goto fetch;
        case 37: putbyte(A, B, rdM(P + 4)); goto fetch;
        case 38: A = input(); goto fetch;
        case 39: A = output(); goto fetch;
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef PAGEDMEM
    int i, status;
    char * buf;
#endif

    if (argc != 2) {
        write(2, "usage: icint file\n",18);
        return 1;
    }
    fp = open(argv[1], O_RDONLY);
    if (fp == -1) {
        perror(argv[1]);
        return 0;
    }

#ifdef PAGEDMEM
    for (i=0; i<PBLKS; i++) {
        pmap[i] = -1; /* start with no physical buffers assigned */
    }

    for (i=0; i<VBLKS; i++) {
        vmap[i] = -1; /* therefore no virtual pages can be assigned */
        tcnt[i] = 0;
        pagstate[i] = PAGNEVER;
        tcnt[i] = 0;
    }

    /* create page file */
    pfp = open("icintv.pag", O_RDWR|O_CREAT|O_TRUNC, 0600);
    if (pfp == -1) {
        perror("icintv.pag");
        return 0;
    }
    buf = (char *)M;
    for (i=0; i<VBLKS; i++) {
        if (2*BLKSIZE != write(pfp, buf, 2*BLKSIZE)) {
            perror("creating empty page file");
            exit(1);
        }
    }
#endif

    G = MGLOB;
    P = MPROG;
    wrM(P++, LIG1);
    wrM(P++, K2);
    wrM(P++, X22);
    initio();
    writes("INTCODE SYSTEM ENTERED\n");
    assemble();
    close(fp);
    writes("\nPROGRAM SIZE = ");
    writes(_itoa(P - MPROG));
    writes("\n");
    C = MPROG;
    return interpret();
}
