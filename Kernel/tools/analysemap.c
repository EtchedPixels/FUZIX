#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int code_len, code2_len, data_len, bss_len, init_len, gsfinal, common_addr,
  initial, discard_base, discard_len;

static int is_discard(int addr)
{
  if (addr < discard_base || addr >= discard_base + discard_len)
    return 0;
  return 1;
}

int main(int argc, char *argv[])
{
  char buf[512];
  int addr = 0;
  int naddr;
  char name[100];
  char nname[100];
  int hogs = 0;
  int all = 0;
  
  if (strstr(argv[0], "memhog"))
    hogs = 1;
  if (argv[1] && strcmp(argv[1], "-a") == 0)
    all = 1;

  
  while(fgets(buf, 511, stdin)) {
    char *p1 = strtok(buf," \t\n");
    char *p2 = NULL;
    int match = 0;
    
    match = memcmp(buf, "     000", 8);

    if (p1)
      p2 = strtok(NULL, " \t\n");

    if (p1 == NULL || p2 == NULL)
      continue;
          
    if (strcmp(p2, "l__CODE") == 0)
      sscanf(p1, "%x", &code_len);
    if (strcmp(p2, "l__CODE2") == 0)
      sscanf(p1, "%x", &code2_len);
    if (strcmp(p2, "l__DATA") == 0)
      sscanf(p1, "%x", &data_len);
    if (strcmp(p2, "l__BSS") == 0)
      sscanf(p1, "%x", &bss_len);
    if (strcmp(p2, "l__INITIALIZED") == 0)
      sscanf(p1, "%x", &init_len);
    if (strcmp(p2, "l__DISCARD") == 0)
      sscanf(p1, "%x", &discard_len);
    if (strcmp(p2, "s__GSFINAL") == 0)
      sscanf(p1, "%x", &gsfinal);
    if (strcmp(p2, "s__INITIALIZER") == 0)
      sscanf(p1, "%x", &initial);
    if (strcmp(p2, "s__COMMONMEM") == 0)
      sscanf(p1, "%x", &common_addr);
    if (strcmp(p2, "s__DISCARD") == 0)
      sscanf(p1, "%x", &discard_base);
      
    if (hogs && match == 0) {
      if (strstr(p2, "_start")) {
        sscanf(p1, "%x", &naddr);
        strcpy(nname, p2);
        if (addr && (all || !is_discard(addr)) ) {
          name[strlen(name)-6]=0;
          printf("%d: %s\n", naddr-addr, name);
        }
        addr = naddr;
        strcpy(name, nname);
      }
    }
  }
  if (!hogs) {
    if (code2_len == 0)
      printf("Code:  %d bytes\n", code_len);
    else {
      printf("Code1: %d bytes\n", code_len);
      printf("Code2: %d bytes\n", code2_len);
      printf("Code:  %d bytes\n", code_len+code2_len);
    }
    printf("Data:  %d bytes\n", data_len);
    printf("BSS:   %d bytes\n", bss_len);
    printf("Initialized: %d bytes\n", init_len);
    printf("Free memory begins at: %04x\n", initial);
    printf("Common is at: %04x\n", common_addr);
    printf("Space: %d bytes\n", common_addr - initial);
    printf("Work room: %d bytes\n", common_addr - initial - init_len);
  }
  exit(0);
}



