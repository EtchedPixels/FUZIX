/* Copyright (c) 2004 Robert Nordier.  All rights reserved. */

/* $Id: icint.c,v 1.6 2004/12/11 12:01:53 rn Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "blib.h"

#define VSIZE 20000
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

static int16_t interpret(void)
{
fetch:
    itrace2("%d = ", C);

    if (C >= VSIZE || P >= VSIZE || C == 0) {
     fprintf(stderr, "FAULT %d %d\n", C, P);
     exit(1);
    }
    W = M[C++];
    if ((W & DBIT) == 0)
        D = W & ABITS;
    else
        D = M[C++];

    itrace3("OP %d, D %d ", W >> FSHIFT, D);

    if ((W & PBIT) != 0) { D += P; itrace("+P"); }
    if ((W & GBIT) != 0) { D += G; itrace("+G"); }
    if ((W & IBIT) != 0) { D = M[D]; itrace("[]"); }
    
    itrace4("->%d [A%dB%d]\n", D, A, B);

    switch (W >> FSHIFT) {
    error: default: writes("\nINTCODE ERROR AT C = ");
                    writes(_itoa(C-1));
                    writes("\n");
        return -1;
    case 0: B = A; A = D; goto fetch;
    case 1: M[D] = A; goto fetch;
    case 2: A = A + D; goto fetch;
    case 3: C = D; goto fetch;
    case 4: A = !A;
    case 5: if (!A) C = D; goto fetch;
    case 6: D += P;
        M[D] = P; M[D + 1] = C;
        P = D; C = A;
        goto fetch;
    case 7: switch (D) {
        default: goto error;
        case 1: A = M[A]; goto fetch;
        case 2: A = -A; goto fetch;
        case 3: A = ~A; goto fetch;
        case 4: C = M[P + 1];
            P = M[P];
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
        case 23: B = M[C]; D = M[C + 1];
            while (B != 0) {
                B--; C += 2;
                if (A == M[C]) { D = M[C + 1]; break; }
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
        case 31: A = M[P]; goto fetch;
        case 32: P = A; C = B; goto fetch;
        case 33: endread(); goto fetch;
        case 34: endwrite(); goto fetch;
        case 35: D = P + B + 1;
                 M[D] = M[P];
                 M[D + 1] = M[P + 1];
                 M[D + 2] = P;
                 M[D + 3] = B;
                 P = D;
                 C = A;
                 goto fetch;
        case 36: A = getbyte(A, B); goto fetch;
        case 37: putbyte(A, B, M[P + 4]); goto fetch;
        case 38: A = input(); goto fetch;
        case 39: A = output(); goto fetch;
        }
    }
}

uint16_t pgvec[VSIZE];

int main(int argc, char *argv[])
{

    if (argc != 2) {
        write(2, "usage: icint file\n",18);
        return 1;
    }
    fp = open(argv[1], O_RDONLY);
    if (fp == -1) {
        perror(argv[1]);
        return 0;
    }
    M = pgvec;
    G = MGLOB;
    P = MPROG;
    M[P++] = LIG1;
    M[P++] = K2;
    M[P++] = X22;
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
