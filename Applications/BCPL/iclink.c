/*
 *	Linker for BCPL modules. We pre-link to keep the interpreter size
 *	much tighter and to up the speed	
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "blib.h"

#define VSIZE 15000
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

uint16_t *M;
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
        M[A] = 0;
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
    if (P >= VSIZE) {
     printf("\nProgram code too large\n");
     exit(1);
    }
    M[P++] = w;
    Cp = 0;
}

static void
stc(uint16_t c)
{
    if (Cp == 0) { stw(0); Cp = WORDSIZE; }
    Cp -= BYTESIZE;
    M[P - 1] += c << Cp;
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
        uint16_t nv = M[kp];
        /* Removing this debug check breaks under SDCC - FIXME check if
           compiler bug ! */
        if (n == 9499)fprintf(stderr, "setlab %d to %d\n", (unsigned int)kp, (unsigned int)P);
        M[kp] = P;
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
    M[a] += k;
}

uint16_t pgvec[VSIZE];

static uint16_t hdr[5] = { 0xBC0D, 0, 0, 0, 0};

int main(int argc, char *argv[])
{

    if (argc != 3) {
        write(2, "usage: iclink cintcode binary\n",30);
        return 1;
    }
    fp = open(argv[1], O_RDONLY);
    if (fp == -1) {
        perror(argv[1]);
        return 1;
    }
    M = pgvec;
    G = MGLOB;
    P = MPROG;
    M[P++] = LIG1;
    M[P++] = K2;
    M[P++] = X22;
    assemble();
    close(fp);
    
    fp = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC);
    if (fp == -1 ) {
     perror(argv[2]);
     return 1;
    }
    hdr[1] = MGLOB;
    hdr[2] = MPROG;
    hdr[3] = P;
    write(fp, hdr, sizeof(hdr));
    if (P >= 16384) {
     write(fp, M, 32766);
     M+= 16383;
     P-= 16383;
    }
    write(fp, M, P * sizeof(uint16_t));
    close(fp);
    return 0;
}
