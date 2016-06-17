#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

static char path[] = "/usr/lib/cpm/emulator.bin";

void writes(const char *p)
{
  write(2, p, strlen(p));
}

/* Stack space is valuable constant space is almost free */
struct stat st;
unsigned char buf[4];
unsigned char *basep;
void (*exec)(void);
char pathbuf[512];
char *pathp;
int fd;
int len, len2;
uint8_t *fcb = (uint8_t *)0x5C;
uint8_t *tail = (uint8_t *)0x81;
uint8_t *tailsize = (uint8_t *)0x80;
char **argp;
int nfcb;

void prepare_fcb(char *p)
{
  int n = 0;
  char c = toupper(*p);
  /* We don't parse user codes : FIXME ? */
  if (p[1] == ':' && c <='A' && c >='Z') {
    *fcb = c - 'A' + 1;
    p += 2;
  }
  while(*p && !isspace(*p) && *p != '.' && n < 8)
    fcb[++n] = *p++;
  if (*p == '.') {
    p++;
    n = 8;
    while(*p && !isspace(*p) && *p != '.' && n < 11)
      fcb[++n] = *p++;
  }
  fcb += 16;
}

int main(int argc, char *argv[])
{

  if (argc < 2) {
    writes("runcpm [app.com] [args...]\n");
    exit(1);
  }

  /* Quick idiot trap */
  if ((uint16_t)main >= 0x1000U) {
    writes("runcpm: low memory base platform needed.\n");
    exit(1);
  }

  fd = open(path, O_RDONLY);
  if (fd == -1) {
    writes("runcpm: emulator.bin missing\n");
    exit(1);
  }
  if (read(fd, buf, 4) != 4 || buf[2] < 0x80) {
    writes("runcpm: emulator.bin invalid\n");  
    exit(1);
  }

  basep = (unsigned char *)(((unsigned int)buf[2]) << 8);

  if (brk(basep + 0xC00)) {
    writes("runcpm: insufficient memory\n");
    exit(1);
  }

  memcpy(basep, buf, 4);
  if (read(fd, basep + 4, 0xBFC) != 0xBFC) {
    writes("runcpm: emulator.bin too short\n");
    exit(1);
  }
  close(fd);

  /* FIXME: worth having a CP/M search path ? */

  pathp = strrchr(argv[1], '/');
  if (pathp == NULL)
    pathp = argv[1];
  pathp = strrchr(argv[1], '.');

  /* Has a .com extension so honour it */
  if (pathp && strcasecmp(pathp, ".com") == 0)
    fd = open(argv[1], O_RDONLY);
  else {
    len = strlen(argv[1]);
    pathp = pathbuf + strlcpy(pathbuf, argv[1], sizeof(pathbuf));
    strlcpy(pathp, ".com", sizeof(pathbuf) - len);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
      strlcpy(pathp, ".COM", sizeof(pathbuf) - len);
      fd = open(path, O_RDONLY);
      if (fd == -1) {
        writes("runcpm: unable to find .COM file\n");
        exit(1);
      }
    }
  }
  if (fd == -1 || fstat(fd, &st) == -1) {
    perror(argv[1]);
    exit(1);
  }
  if (!S_ISREG(st.st_mode) || st.st_size < 256) {
    writes(argv[1]);
    writes(" does not appear to be a CP/M .COM file\n");
    exit(1);
  }
  exec = (void *)basep;
  /* Make sure we pass the binary as fd 3 */
  if (fd != 3) {
    dup2(fd, 3);
    close(fd);
  }
  /* Now build the FCB and argument area in C so we can keep as much in portable
     discardable code as we can */
  memset(fcb, 0, 16);
  memset(fcb + 1, ' ', 11);

  argp = argv + 2;
  len = 0;

  /* Linearize arguments that will fit */
  while(*argp) {
    len2 = strlen(*argp);
    if (len + len2 > 126)
      break;
    if (nfcb++ < 2)
      prepare_fcb(*argp);
    memcpy(tail, *argp, len2);
    tail += len2;
    *tail++ = ' ';
    len += len2 + 1;
    argp++;
  }
  *tailsize = len;

  /* If this works it blows us away and we never return */
  (*exec)();
  writes("runcpm: exec failure\n");
  exit(1);
}
