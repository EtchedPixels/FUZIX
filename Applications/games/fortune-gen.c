#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static char buf[256];
static int n;
static off_t off;

static void write_record(void)
{
  uint32_t o = htonl(off);
  if (fseek(stdout, 2 + 4 * n, 0) == -1) {
    perror("fseek");
    exit(1);
  }
  if (fwrite(&o, 4, 1, stdout) != 1) {
    perror("fwrite1");
    exit(1);
  }
  if (fseek(stdout, off, 0) == -1) {
    perror("fseek");
    exit(1);
  }
  n++;
}

int main(int argc, char *argv[])
{
  uint16_t count = 0;

  while(fgets(buf, 255, stdin)) {
    if (strcmp(buf, "%\n") == 0)
      count++;
  }

  fprintf(stderr, "%d fortunes.\n", (int)count);

  if (fseek(stdin, 0L, 0) == -1) {
    perror("fseek");
    exit(1);
  }
  
  off = 2 + 4 * count;
  write_record();

  while(fgets(buf, 255, stdin)) {
    if (strcmp(buf, "%\n") == 0) {
      write_record();
    } else {
      if (fwrite(buf, strlen(buf), 1, stdout) != 1) {
        perror("fwrite2");
        exit(1);
      }
      off += strlen(buf);
    }
  }
  write_record();
  if (fseek(stdout, 0L, 0) == -1) {
    perror("fseek");
    exit(1);
  }
  buf[0] = count & 0xff;
  buf[1] = count >> 8;
  if (fwrite(&buf, 2, 1, stdout) != 1) {
    perror("fwrite3");
    exit(1);
  }
  return 0;
}
