/*
 *	Fix up the given binary block with the reloc data
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#define NBANKS	5

unsigned char buf[NBANKS][65536];
unsigned int size[NBANKS];
FILE *fptr[NBANKS];
int v = 0;
static int nextfix;
static int lastfix;
static int stub_all;

struct symbol {
  struct symbol *next;
  char *name;
  unsigned int val;
};

struct symbol *symbols;

struct stubmap {
  struct stubmap *next;
  int sbank;
  int dbank;
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

void add_symbol(char *name, unsigned int val)
{
  struct symbol *s = malloc(sizeof(struct symbol) + strlen(name) + 1);
  if (s == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  s->name = (char *)(s + 1);
  s->val = val;
  strcpy(s->name, name);
  s->next = symbols;
  symbols = s;
}

unsigned int find_symbol(const char *name)
{
   struct symbol *s = symbols; 
   while (s) {
     if (strcmp(s->name, name) == 0)
       return s->val;
     s = s->next;
  }
  fprintf(stderr, "Symbol '%s' is missing for banked use.\n", name);
  exit(1);
}
  
unsigned int get_bank_function(int sbank, int dbank)
{
   char name[32];
   sprintf(name, "__bank_%d_%d", sbank, dbank);
   return find_symbol(name);
}

unsigned int get_stub_function(int sbank, int dbank)
{
   char name[32];
   sprintf(name, "__stub_%d_%d", sbank, dbank);
   return find_symbol(name);
}

int stubmap(uint16_t v, int sbank, int dbank)
{
  struct stubmap *s = stubs;
  struct stubmap **p = &stubs;
  uint16_t da;

  while(s) {
    if (s->sbank == sbank && s->addr == v && s->dbank == dbank) {
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
  s->sbank = sbank;
  s->dbank = dbank;
  s->addr = v;
  s->target = nextfix;
  if (nextfix >= lastfix) {
    fprintf(stderr, "Out of fix space (%d stubs used).\n", stubct);
    exit(1);
  }
  /* FIXME: we could have per bank stubs in ROM and not waste precious
     common memory here */
  da = get_stub_function(sbank, dbank);
  /* LD DE, targetaddr */
  buf[0][nextfix++] = 0x11;
  buf[0][nextfix++] = v & 0xFF;
  buf[0][nextfix++] = v >> 8;
  /* JP */
  buf[0][nextfix++] = 0xC3;
  /* stub helper */
  buf[0][nextfix++] = da & 0xFF;
  buf[0][nextfix++] = da >> 8;
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
    case 0x01:	/* LD BC, */
    case 0x11:	/* LD DE, */
    case 0x21:	/* LD HL/IX/IY, */
    case 0xC3:	/* JP - needs stub */
      if (v)
        printf("Converting %02x at %04x to stub\n", buf[sbank][ptr-1], ptr);
      da = stubmap(buf[sbank][ptr] + (buf[sbank][ptr+1] << 8), sbank, dbank);
      buf[sbank][ptr] = da & 0xFF;
      buf[sbank][ptr+1] = da >> 8;
      break;
    case 0xCD:	/* PUSH AF CALL POP AF */
      if (buf[sbank][ptr-2] != 0xF5|| buf[sbank][ptr+2] != 0xF1) {
        fprintf(stderr, "Bad format for relocated long call at %04x (%02x %02x %02x %02x %02x\n",
          ptr, buf[sbank][ptr-2], buf[sbank][ptr-1], buf[sbank][ptr], buf[sbank][ptr+1], buf[sbank][ptr+2]);
        exit(1);
      }
      if (v)
        printf("Converting CALL at %04x from bank %d to bank %d\n", ptr,
          sbank, dbank);
      /* Turn the push af into a call */
      buf[sbank][ptr-2] = 0xCD;
      /* Move the address along */
      buf[sbank][ptr+2] = buf[sbank][ptr+1];
      buf[sbank][ptr+1] = buf[sbank][ptr];
      /* Fit in the actual call target */
      da = get_bank_function(sbank, dbank);
      /* Sequence is now CALL __bank_sbank_dbank DW target */
      buf[sbank][ptr-1] = da & 0xFF;
      buf[sbank][ptr] = da >> 8;
      break;
    default:
      fprintf(stderr, "Bad relocation in code (%02X)%04X: %02X\n",
        sbank, ptr-1, buf[sbank][ptr-1]);
  }
}

void data_reloc(uint8_t sbank, uint16_t ptr, uint8_t dbank)
{
  int na;
  uint16_t n;
  
  if (dbank == 0)
    return;
  /* Get the target */
  n = buf[sbank][ptr] + (buf[sbank][ptr+1]<<8);

  if (v)
    printf("Stubhooking %04x for data reference.\n", ptr);  
  /* Find the stub for it */
  na = stubmap(n, sbank, dbank);
  if (na == -1) {
    fprintf(stderr, "No stub match: stubs stale\n");
    exit(1);
  }
  /* Patch in the revised destination address */
  buf[sbank][ptr] = na & 255;
  buf[sbank][ptr+1] = na >> 8;
  if (v)
    printf("Patched %04x to %04x\n", ptr, na);
}

int stub_code(char *name)
{
  if(strncmp(name, "_CODE", 5) == 0)
    return 1;
  if(strcmp(name, "_VIDEO") == 0)
    return 1;
  if(strcmp(name, "_COMMONMEM") == 0)
    return 1;
  if(strncmp(name, "_DISCARD", 8) == 0)
    return 1;
  if(strncmp(name, "_BOOT", 5) == 0)
    return 1;
  /* Data */
  if(strcmp(name, "_COMMONDATA") == 0)
    return 0;
  if(strcmp(name, "_INITIALIZER") == 0)
    return 0;
  if(strncmp(name, "_DATA", 5) == 0)
    return 0;
  if(strncmp(name, "_BUFFERS", 8) == 0)
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
  char sname[65];

  if (strlen(p) > 4 && !isspace(p[8]))
    fprintf(stderr, "Overflow: %s", p);
  if (sscanf(p, "%02x %04x %02x %64s %64s", &b1, &addr, &b2, name, sname) != 5) {
    fprintf(stderr, "Invalid relocation link %s\n", p);
    exit(1);
  }
  /* A data relocation into another bank of data is treated as deliberate.
     We can't do much else. It's not code so we can't patch it, and if it's
     a data pointer (as it should be) then a relocation is nonsense! */
  if (!stub_code(sname) && !stub_code(name))
    return;
  /* If we are stubbing the lot then code is handled as data is */
  if (stub_code(name) && !stub_all)
    code_reloc(b1, addr, b2);
  else
    data_reloc(b1, addr, b2);
}

static void scan_symbols(FILE *f)
{
  char buf[256];
  char *sym;
  unsigned int addr;

  while(fgets(buf, 255, f)) {
    if (memcmp(buf, "     0000", 9))
      continue;
    /* Looks like a symbol */
    if (sscanf(buf+5, "%x", &addr) != 1)
      continue;
    /* Smells like a symbol */
    sym = strtok(buf+15, " \n\t");
    if (!sym)
      continue;
    /* Guess it's a symbol then */
    if (v && (strstr(sym, "_bank_") || strstr(sym, "_stub_")))
      printf("Add symbol %s %x\n", sym, addr);
    add_symbol(sym, addr);
  }
}    

static void move_initializers(void)
{
  unsigned int s_initsrc;
  unsigned int s_initdst;
  unsigned int l_init;

  s_initsrc = find_symbol("s__INITIALIZER");
  s_initdst = find_symbol("s__INITIALIZED");
  l_init = find_symbol("l__INITIALIZER");
  if (v)
    printf("Moving %d bytes from %x to %x for initialization\n",
      l_init, s_initsrc, s_initdst);
  memcpy(buf[0] + s_initdst, buf[0] + s_initsrc, l_init);
}

static void zero_data(void)
{
  unsigned int seg, len;
  seg = find_symbol("s__DATA");
  len = find_symbol("l__DATA");
  if (v)
    printf("Zeroing %x for %d\n", seg, len);
  memset(buf[0] + seg, 0, len);
  seg = find_symbol("s__BSS");
  len = find_symbol("l__BSS");
  if (v)
    printf("Zeroing %u for %d\n", seg, len);
  memset(buf[0] + seg, 0, len);
}

int main(int argc, char *argv[])
{
  FILE *r;
  char in[256];
  char bin[64];
  int banks;
  int sb, ss;
  int opt;

  while ((opt = getopt(argc, argv, "av")) != -1) {
    switch (opt) {
      case 'a':
        stub_all = 1;
        break;
      case 'v':
        v = 1;
        break;
    }
  }
  if (argc != optind + 1) {
    fprintf(stderr, "%s -v stubstart-size\n", argv[0]);
    exit(1);
  }
  if (sscanf(argv[optind], "%x-%x", &sb, &ss) != 2) {
    fprintf(stderr, "%s: invalid stub info\n", argv[0]);
    exit(1);
  }

  nextfix = sb;
  lastfix = sb + (ss & ~3);
  
  r = fopen("fuzix.map", "r");
  if (r == NULL) {
    perror("fuzix.map");
    exit(1);
  }
  scan_symbols(r);
  fclose(r);
  
 
  r = fopen("relocs.dat", "r");
  if (r == NULL) {
    perror("relocs.dat");
    exit(1);
  }

  for (banks = 0; banks < NBANKS; banks ++) {
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

  move_initializers();
  zero_data();		/* For SNA mode */

  for (banks = 0; banks < NBANKS; banks++) {
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
    stubct, stubct * 6, stubdup);
  exit(0);
}
