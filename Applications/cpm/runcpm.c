#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

static char path[] = "/usr/lib/cpm/emulator.bin";

void writes(const char *p)
{
  write(2, p, strlen(p));
}

static struct stat st;
unsigned char buf[4];
unsigned char *basep;
void (*exec)(char *argv[]);

int main(int argc, char *argv[])
{

  int fd;
  
  if (argc < 2) {
    writes("runcpm [app.com] [args...]\n");
    exit(1);
  }
  
  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror(path);
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
  if (read(fd, basep + 4, 0x2FC) != 0x2FC) {
    writes("runcpm: emulator.bin too short\n");
    exit(1);
  }
  close(fd);

  fd = open(argv[1], O_RDONLY);
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
  /* If this works it blows us away and we never return */
  (*exec)(argv);
  writes("runcp: exec failure\n");
  exit(1);
}

  