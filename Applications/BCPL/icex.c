/* Copyright (c) 2004 Robert Nordier.  All rights reserved. */

/* $Id: icint.c,v 1.6 2004/12/11 12:01:53 rn Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "blib.h"

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

#define VSIZE (20 * 1024)
uint16_t M[VSIZE];
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

static void stw(uint16_t);
static void stc(uint16_t);
static void rch(void);
static int16_t rdn(void);
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


static int16_t interpret(void)
{
fetch:
    itrace2("%d = ", C);

    if (C >= VSIZE || P >= VSIZE || C == 0) {
     writes("\nFAULT ");
     writes(_itoa(C));
     writes(" ");
     writes(_itoa(P));
     writes("\n");
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

static uint16_t hdr[5];

int main(int argc, char *argv[])
{

    if (argc != 2) {
        write(2, "usage: icex file\n",18);
        return 1;
    }
    fp = open(argv[1], O_RDONLY);
    if (fp == -1) {
        perror(argv[1]);
        return 0;
    }
    read(fp, hdr, sizeof(hdr));
    
    G = hdr[1];
    C = hdr[2];
    P = hdr[3];

    if (P >= 16384) {
     read(fp, M, 32766);
     read(fp, M + 16383, P - 32766);
    } else 
     read(fp, M, P * sizeof(uint16_t));
    close(fp);
    initio();
    return interpret();
}
