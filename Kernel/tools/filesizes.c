#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int code_len, code2_len, data_len, bss_len, init_len, gsfinal, common_addr,
  initial;

int main(int argc, char *argv[])
{
  char buf[512];
  char fname[100];
  unsigned int addr, naddr;  
  *fname = 0;
  
  while(fgets(buf, 511, stdin)) {
    char *p1 = strtok(buf," \t\n");
    char *p2;
    char *p3;
    int match = 0;
    
    match = memcmp(buf, "     000", 8);

    if (p1)
      p2 = strtok(NULL, " \t\n");
    if (p2)
      p3 = strtok(NULL, " \t\n");

    if (match || p1 == NULL || p2 == NULL || p3 == NULL)
      continue;
    
    if (strcmp(p3, fname)) {
      if (sscanf(p1, "%x", &naddr) == 0)
        fprintf(stderr, "Bad address '%s'\n", p1);
      if (*fname)
        printf("%-24s: %d\n", fname, naddr-addr);
      addr = naddr;
      strcpy(fname, p3);
    }
  }
  exit(0);
}



