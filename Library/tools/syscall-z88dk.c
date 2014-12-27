/*
 *	Generate the syscall functions for Z88DK
 *
 *	Z88DK has some "interesting" properties. It's SmallC derived
 *	so pushes the arguments backwards which we must cope with. It also
 *	has a nifty 'fastcall' HL single argument passing ABI that you can
 *	use (SDCC take notes please).
 *
 *	We turn 2/3/4 argument syscalls into stack shuffles (in and out) and
 *	single argument into fastcall.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscall_name.h"

static char namebuf[128];

static void write_call(int n)
{
  FILE *fp;
  snprintf(namebuf, 128, "z88dk/%s.asm", syscall_name[n]);
  fp = fopen(namebuf, "w");
  if (fp == NULL) {
    perror(namebuf);
    exit(1);
  }
  fprintf(fp, "\tLIB syscall\n");
  fprintf(fp, "\tXLIB %s\n\n", syscall_name[n]);
  fprintf(fp, ".%s\n", syscall_name[n]);

  switch(syscall_args[n]) {
    case 0:  
      fprintf(fp, "\tld hl, %d\n", n);
      fprintf(fp, "\tjp syscall\n");
      break;
    case 1:
      fprintf(fp, "\tpush hl\n");
      fprintf(fp, "\tld hl, %d\n", n);
      fprintf(fp, "\tcall syscall\n");
      fprintf(fp, "\tpop af\n");
      fprintf(fp, "\tret\n");
      break;
    case 2:
      fprintf(fp, "\tpop hl\n\tpop de\n\tpop bc\n\tpush de\n\tpush bc\n\tpush hl\n");
      fprintf(fp, "\tld hl, %d\n", n);
      fprintf(fp, "\tcall syscall\n");
      fprintf(fp, "\tpop ix\n\tpop de\n\tpop bc\n\tpush de\n\tpush bc\n");
      fprintf(fp, "\tjp (ix)\n");
      break;
    case 3:
      fprintf(fp, "\tpop ix\n\tpop de\n\tpop bc\n\tpop hl\n\tpush de\n\tpush bc\n\tpush hl\n\tpush ix\n");
      fprintf(fp, "\tld hl, %d\n", n);
      fprintf(fp, "\tcall syscall\n");
      fprintf(fp, "\tpop ix\n\tpop de\n\tpop bc\n\tpop iy\n\tpush de\n\tpush bc\n\tpush iy\n");
      fprintf(fp, "\tjp (ix)\n");
      break;
    case 4:
      fprintf(fp, "\tpop ix\n\tpop de\n\tpop bc\n\tpop hl\n\tpop iy\n\tpush de\n\tpush bc\n\tpush hl\n\tpush iy\n\tpush ix\n");
      fprintf(fp, "\tld hl, %d\n", n);
      fprintf(fp, "\tcall syscall\n");
      fprintf(fp, "\tpop ix\n\tpop de\n\tpop bc\n\tpop iy\n\tpop af\n\tpush de\n\tpush bc\n\tpush iy\n\tpush af\n");
      fprintf(fp, "\tjp (ix)\n");
      break;
  }
  fclose(fp);
}

static void write_header(FILE *fp, int n)
{
  fprintf(fp, "extern int __LIB__ ");
  if (syscall_args[n] == 1)
    fprintf(fp, "__FASTCALL__ ");
  fprintf(fp, "%s(", syscall_name[n]);
  if (syscall_args[n] == 0)
    fprintf(fp, "void);\n");
  if (syscall_args[n] == 1)
    fprintf(fp, "unsigned int);\n");
  if (syscall_args[n] == 2)
    fprintf(fp, "char *, unsigned int);\n");
  if (syscall_args[n] == 3)
    fprintf(fp, "unsigned int, void *, unsigned int);\n");
  if (syscall_args[n] == 4)
    fprintf(fp, "unsigned int, unsigned int, unsigned int, unsigned int);\n");
}


static void write_call_table(void)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++)
    write_call(i);
}

static void write_proto_header(void)
{
  int i;
  FILE *fp = fopen("z88dk/header.proto", "w");
  if (fp == NULL) {
    perror("z88dk/header.proto");
    exit(1);
  }
  for (i = 0; i < NR_SYSCALL; i++)
    write_header(fp, i);
  fclose(fp);
}

int main(int argc, char *argv[])
{
  write_proto_header();
  write_call_table();
  exit(0);
}
