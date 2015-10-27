#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/syscall_name.h"

int syscall_bank[NR_SYSCALL];

void located_function(const char *func, int code)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++) {
    if (strcmp(func, syscall_name[i]) == 0 ||
        strcmp(func + 1, syscall_name[i]) == 0) {
      if (syscall_bank[i])
        fprintf(stderr, "%s is in bank %d and %d\n", func, code, 
                          syscall_bank[i]);
      else
        syscall_bank[i] = code;
      return;
    }
  }
  fprintf(stderr, "%s is confusing me\n", func);
}

int scan_c_file(const char *name, int code)
{
  char buf[256];
  char *t;
  FILE *f = fopen(name, "r");
  if (!f) {
    perror(name);
    exit(1);
  }
  while(fgets(buf, 256, f)) {
    if (memcmp(buf, "arg_t _", 7))
      continue;
    t = strtok(buf + 6, " (\t\n");
    if (t == NULL)
      continue;
    located_function(t, code);
  }
  fclose(f);
}

void output_banks(void)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++)
    printf("\t%d, /* %s */\n", syscall_bank[i], syscall_name[i]);
}

int main(int argc, char *argv[])
{
  int i = 1;
  while (*++argv)
    scan_c_file(*argv, i++);
  output_banks();
  return 0;
}
