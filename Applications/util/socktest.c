#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define AF_INET		1
#define SOCK_STREAM	3
#define IPPROTO_TCP	6

struct in_addr {
  uint32_t s_addr;
};

struct sockaddr_in {
  uint16_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

struct sockaddr_in addr = {
  AF_INET,
  0xEA1D,
  { 0xF400A8C0 },
};

struct sockaddr_in laddr = {
  AF_INET,
  0x0A0A,
  { 0x010000C0 },
};

int main(int argc, char *argv[])
{
  int fd;
  int r;
  char buf[5];
  
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0)
    perror("af_inet sock_stream pf_tcp");
  close(fd);
      
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("af_inet sock_stream 0");
    exit(1);
  }
  if (bind(fd, (struct sockaddr *)&laddr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }
  
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }
  if (write(fd, "Hello", 5) == -1) {
    perror("write");
    exit(1);
  }
  if ((r = read(fd, buf, 5)) < 0) {
    perror("read");
    exit(1);
  }
  printf("Read %d bytes\n", r);
  write(1, buf, r);
  close(fd);
  return 0;      
}
