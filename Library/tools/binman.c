#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *	This is a close relative of the kernel binman but produces
 *	user space binaries and doesn't have common packing magic or all
 *	the magic segments in the kernel
 */

static uint8_t buf[65536];

static unsigned int s__INITIALIZER, s__INITIALIZED;
static unsigned int l__INITIALIZER;

static unsigned int s__DATA, l__DATA;

static unsigned int progload = 0x100;
                      

static void ProcessMap(FILE *fp)
{
  char buf[512];
  
  while(fgets(buf, 511, fp)) {
    char *p1 = strtok(buf," \t\n");
    char *p2 = NULL;
    
    if (p1)
      p2 = strtok(NULL, " \t\n");

    if (p1 == NULL || p2 == NULL)
      continue;
          
    if (strcmp(p2, "s__DATA") == 0)
      sscanf(p1, "%x", &s__DATA);
    if (strcmp(p2, "l__DATA") == 0)
      sscanf(p1, "%x", &l__DATA);
    if (strcmp(p2, "s__INITIALIZED") == 0)
      sscanf(p1, "%x", &s__INITIALIZED);
    if (strcmp(p2, "s__INITIALIZER") == 0)
      sscanf(p1, "%x", &s__INITIALIZER);
    if (strcmp(p2, "l__INITIALIZER") == 0)
      sscanf(p1, "%x", &l__INITIALIZER);
 }
}


int main(int argc, char *argv[])
{
  FILE *map, *bin;
  uint8_t *bp;

  if (argc != 5) {
    fprintf(stderr, "%s: <PROGLOAD address> <binary> <map> <output>\n", argv[0]);
    exit(1);
  }

  if (sscanf(argv[1], "%x", &progload) != 1)
  {
    fprintf(stderr, "%s: PROGLOAD parse error: %s\n", argv[0], argv[1]);
    exit(1);
  }

  bin = fopen(argv[2], "r");
  if (bin == NULL) {
    perror(argv[2]);
    exit(1);
  }
  if (fread(buf, 1, 65536, bin) == 0) {
    fprintf(stderr, "%s: read error on %s\n", argv[0], argv[2]);
    exit(1);
  }
  fclose(bin);
  map = fopen(argv[3], "r");
  if (map == NULL) {
    perror(argv[3]);
    exit(1);
  }
  ProcessMap(map);
  fclose(map);
  
  bin = fopen(argv[4], "w");
  if (bin == NULL) {
    perror(argv[4]);
    exit(1);
  }

  if (s__INITIALIZER + l__INITIALIZER > 65535 || s__INITIALIZED + l__INITIALIZER > 65535 || s__DATA > 65535) {
    fprintf(stderr, "%s: too large for this model.\n", argv[0]);
    /* FIXME: but for now it makes the build a hassle otherwise */
    exit(0);
  }
  memcpy(buf + s__INITIALIZED, buf + s__INITIALIZER, l__INITIALIZER);

  if (progload & 0xFF) {
    fprintf(stderr, "%s: load address must be page aligned.\n", argv[0]);
    exit(1);
  }

  bp = buf + progload + 4;
  *bp++ = progload >> 8;		/* Base page to load */
  bp++;					/* Skip hints */
  *bp++ = s__INITIALIZED - progload;
  *bp++ = (s__INITIALIZED - progload) >> 8;
  *bp++ = s__DATA - s__INITIALIZED;
  *bp++ = (s__DATA - s__INITIALIZED) >> 8;
  *bp++ = l__DATA;
  *bp = l__DATA >> 8;

  /* Write out everything that is data, omit everything that will 
     be zapped */
  if (fwrite(buf + progload, s__DATA - progload, 1, bin) != 1) {
   perror(argv[4]);
   exit(1);
  }
  fclose(bin);
  printf("%s: %d bytes from %d\n", argv[4], s__DATA - progload, progload);
  exit(0);
}
