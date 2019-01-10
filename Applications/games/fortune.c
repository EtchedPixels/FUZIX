#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define COOKIE_JAR	"/usr/games/fortune.dat"

struct f_off {
  uint32_t off;
  uint32_t next;
};


static uint8_t buf[512];
static uint16_t cookies;
static uint16_t cookie;

struct f_off off;

int main(int argc, char *argv[])
{
  int fd = open(COOKIE_JAR, O_RDONLY);
  int n;
  int r;
  int len;

  if (fd == -1) {
    perror(COOKIE_JAR);
    exit(1);
  }
  
  if (read(fd, &buf, 2) != 2) {
    perror("read");
    exit(1);
  }
  cookies = buf[0] + 256 * buf[1];

  srand(getpid() ^ getuid() ^ (uint16_t)time(NULL));
  cookie = rand() % cookies;

  if (lseek(fd, 2 + 4 * cookie, SEEK_SET) == -1) {
    perror("lseek");
    exit(1);
  }
  
  if (read(fd, &off, 8) != 8) {
    perror("read2");
    exit(1);
  }

  off.off = ntohl(off.off);

  if (lseek(fd, off.off, SEEK_SET) == -1) {
    perror("lseek2");
    exit(1);
  }
  
  off.next = ntohl(off.next);

  len = off.next - off.off;

  while (len) {
    n = 512;
    if (len < 512)
      n = len;
    r = read(fd,buf, n);
    if (r != n) {
      perror("read3");
      exit(1);
    }
    write(1, buf, r);
    len -=r;
  }
  close(fd);
  exit(0);
}

    
