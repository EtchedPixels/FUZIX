/*
 *	Generate the syscall functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscall_name.h"

static char namebuf[128];

static void write_call(int n)
{
  FILE *fp;
  snprintf(namebuf, 128, "fuzixmsp430x/syscall_%s.s", syscall_name[n]);
  fp = fopen(namebuf, "w");
  if (fp == NULL) {
    perror(namebuf);
    exit(1);
  }
  fprintf(fp, "\t.text\n\n"
	      "\t.globl _syscall\n"
	      "\t.globl %1$s\n\n"
	      "%1$s:\n", syscall_name[n]);
  fprintf(fp, "\tmov #%d, r11\n"
              "\tbr #_syscall\n",
			  n);
  fclose(fp);
}

static void write_call_table(void)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++)
    write_call(i);
}

int main(int argc, char *argv[])
{
  write_call_table();
  exit(0);
}
