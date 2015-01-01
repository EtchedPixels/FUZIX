/*
 *	Fix up the given binary block with the reloc data
 *
 *	Patch RST lines into the relevant places, generate stub info
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define NBANKS	4

unsigned char buf[NBANKS][65536];
unsigned int size[NBANKS];
FILE *fptr[NBANKS];
int v;
static int nextfix;
static int lastfix;

struct stubmap {
  struct stubmap *next;
  int bank;
  uint16_t addr;
  uint16_t target;
};

struct stubmap *stubs;
static int stubct = 0;
static int stubdup = 0;

unsigned int resize(int b)
{
  unsigned char *bp = buf[b] + 65535;
  while(*bp == 0)
    bp--;
  bp++;
  if (bp - buf[b] > size[b])
    return bp - buf[b];
  return size[b];
}

/* FIXME: */
int stubmap(uint16_t v, int bank)
{
  struct stubmap *s = stubs;
  struct stubmap **p = &stubs;

  while(s) {
    if (s->bank == bank && s->addr == v) {
      stubdup++;
      return s->target;
    }
    p = &s->next;
    s = *p;
  }
  s = malloc(sizeof(struct stubmap));
  if (s == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  *p = s;
  s->next = NULL;
  s->bank = bank;
  s->addr = v;
  s->target = nextfix;
  if (nextfix == lastfix) {
    fprintf(stderr, "Out of fix space (%d stubs used).\n", stubct);
    exit(1);
  }
  buf[0][nextfix++] = 0xC7 + 8 * bank;	/* RST */
  buf[0][nextfix++] = v & 0xFF;	/* Target */
  buf[0][nextfix++] = v >> 8;
  buf[0][nextfix++] = 0xC9;	/* RET */
  stubct++;
  return s->target;
}

void code_reloc(uint8_t sbank, uint16_t ptr, uint8_t dbank)
{
  int da;
  if (ptr == 0) {
    fprintf(stderr, "Nonsense zero based code relocation.\n");
    exit(1);
  }
  if (dbank == 0 || dbank >= NBANKS) {
    fprintf(stderr, "Invalid bank %d\n", dbank);
    exit(1);
  }
  switch(buf[sbank][ptr-1]) {
    case 0xC3:	/* JP - needs stub */
      if (v)
        printf("Converting JP at %04x to RST/RET stub\n", ptr);
      da = stubmap(buf[sbank][ptr] + (buf[sbank][ptr+1] << 8), dbank);
      buf[sbank][ptr] = da & 0xFF;
      buf[sbank][ptr+1] = da >> 8;
      break;
    case 0xCD:	/* CALL */
      if (v)
        printf("Converting CALL at %04x to RST\n", ptr);
      buf[sbank][ptr-1] = 0xC7 + 8 * dbank;	/* RST 8/16/24 */
      break;
    case 0xC7:	/* RST */
    case 0xCF:
    case 0xD7:
    case 0xDF:
      fprintf(stderr, "File already processed!\n");
      exit(1);
    default:
      fprintf(stderr, "Bad relocation in code %04X: %02X\n",
        ptr-1, buf[sbank][ptr-1]);
  }
}

void data_reloc(uint8_t sbank, uint16_t ptr, uint8_t dbank)
{
  int na;
  uint16_t n;
  
  /* Get the target */
  n = buf[sbank][ptr] + (buf[sbank][ptr+1]<<8);

  if (v)
    printf("Stubhooking %04x for data reference.\n", ptr);  
  /* Find the stub for it */
  na = stubmap(n, dbank);
  if (na == -1) {
    fprintf(stderr, "No stub match: stubs stale\n");
    exit(1);
  }
  /* Patch in the revised destination address */
  buf[sbank][ptr] = na & 255;
  buf[sbank][ptr+1] = na >> 8;
}

int stub_code(char *name)
{
  if(strncmp(name, "_CODE", 5) == 0)
    return 1;
  if(strcmp(name, "_VIDEO") == 0)
    return 1;
  if(strcmp(name, "_COMMONMEM") == 0)
    return 1;
  if(strcmp(name, "_DISCARD") == 0)
    return 1;
  if(strncmp(name, "_BOOT", 5) == 0)
    return 1;
  /* Data */
  if(strcmp(name, "_INITIALIZER") == 0)
    return 0;
  if(strcmp(name, "_DATA") == 0)
    return 0;
  if(strcmp(name, "_FONT") == 0)
    return 0;
  if(strcmp(name , "_CONST") == 0)
    return 0;
  fprintf(stderr, "Unknown bank name %s\n", name);
  exit(1);
}

static void process_stub(char *p)
{
  int b1, b2, addr;
  char name[65];
  if (sscanf(p, "%02x %04x %02x %64s", &b1, &addr, &b2, name) != 4) {
    fprintf(stderr, "Invalid relocation link %s\n", p);
    exit(1);
  }
  if (stub_code(name))
    code_reloc(b1, addr, b2);
  else
    data_reloc(b1, addr, b2);
}


int main(int argc, char *argv[])
{
  FILE *r;
  char in[256];
  char bin[64];
  int banks;
  int sb, ss;

  if (argv[1] && strcmp(argv[1], "-v") == 0)
    v = 1;
  if (argc != 2 + v) {
    fprintf(stderr, "%s -v stubstart-size\n", argv[0]);
    exit(1);
  }
  if (sscanf(argv[1+v], "%x-%x", &sb, &ss) != 2) {
    fprintf(stderr, "%s: invalid stub info\n", argv[0]);
    exit(1);
  }

  nextfix = sb;
  lastfix = sb + (ss & ~3);
  
  r = fopen("relocs.dat", "r");
  if (r == NULL) {
    perror("relocs.dat");
    exit(1);
  }

  for (banks = 0; banks < 4; banks ++) {  
    if (banks == 0)
    strcpy(bin, "common.bin");
    else
      sprintf(bin, "bank%d.bin", banks);
    fptr[banks] = fopen(bin, "r+");
    if (fptr[banks] == NULL) {
      if (errno != ENOENT) {
        perror(bin);
        exit(1);
      }
    } else {
      size[banks] = fread(buf[banks], 1, 65536, fptr[banks]);
      if (size[banks] < 1) {
        perror(bin);
        exit(1);
      }
      rewind(fptr[banks]);
    }
  }
  
  while(fgets(in, 256, r)) {
    if (*in != 'B') {
      fprintf(stderr, "Bad record: %s", in);
      exit(1);
    }
    if (v)
      printf("|%s", in);
    process_stub(in + 1);
  }
  fclose(r);

  for (banks = 0; banks < 4; banks++) {
    if (fptr[banks]) {
      /* Just conceivably we might reloc a trailing zero byte and need to grow the
       file */
      size[banks] = resize(banks);
      if (fwrite(buf[banks], size[banks], 1, fptr[banks]) != 1) {
        perror("fwrite");
        exit(1);
      }
      fclose(fptr[banks]);
    }
  }
  printf("%d stub relocations using %d bytes, %d duplicates\n",
    stubct, stubct * 4, stubdup);
  exit(0);
}
