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

  if (syscall_args[n] == VARARGS)
  {
	/* Varargs syscalls have the first argument in r12 and the others on
	 * the stack. We support up to four parameters. */
	fprintf(fp, "\tmov 2(sp), r13\n"
	            "\tmov 4(sp), r14\n"
				"\tmov 6(sp), r15\n");
  }

  /* On entry, the four parameters are in r12-r15. The syscall number is on
   * the stack. */

  fprintf(fp, "\tpush #%d\n"
              "\tpush #_syscall_return\n"
			  "\tbr &(_start-2)\n",
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
