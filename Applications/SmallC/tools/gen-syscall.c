/*
 *	Generate the syscall functions. For Z80 at the moment. Extend to
 *	do the others as well
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscall_name.h"

static char namebuf[128];

/*
 *	For Z80 it's kind of simple - load the call number and jump
 *	to the common helper.
 */
static void write_Z80_call(int n)
{
  FILE *fp;
  snprintf(namebuf, 128, "Z80/syscall/_%s.s", syscall_name[n]);
  fp = fopen(namebuf, "w");
  if (fp == NULL) {
    perror(namebuf);
    exit(1);
  }
  fprintf(fp, "\t.code\n\n");
  fprintf(fp, "\t.export _%s\n\n", syscall_name[n]);
  fprintf(fp, "_%s:\n\tld hl, %d\n", syscall_name[n], n);
  fprintf(fp, "\tjp __syscall\n");
  fclose(fp);
}

static void write_Z80_call_table(void)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++)
    write_Z80_call(i);
}

static void write_Z80_makefile(void)
{
  int i;
  FILE *fp = fopen("Z80/syscall/Makefile", "w");
  if (fp == NULL) {
    perror("Makefile");
    exit(1);
  }
  fprintf(fp, "SRCS = syscall.s\n", syscall_name[0]);
  for (i = 0; i < NR_SYSCALL; i++)
    fprintf(fp, "SRCS += _%s.s\n", syscall_name[i]);
  fprintf(fp, "\nsyscall.a: $(SRCS)\n");
  fprintf(fp, "\t$(CROSS_AR) rc syslib.lib $(OBJS)\n\n");
  fprintf(fp, "$(OBJS): %%.o: %%.s\n");
  fprintf(fp, "\t$(CROSS_AS) $<\n\n");
  fprintf(fp, "clean:\n");
  fprintf(fp, "\trm -f $(OBJS) $(SRCS) syscall.a *~\n\n");
  fclose(fp);
}

int main(int argc, char *argv[])
{
  write_Z80_makefile();
  write_Z80_call_table();
  exit(0);
}
