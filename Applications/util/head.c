/*
 *	Head-light: A stdio free implementation of the head command for Fuzix
 *	Alan Cox (c) 2015. GPLv2
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static char outbuf[512];
static char *outp;

static char buffer[512];

static const char *n;
static int lines = 10;	/* Default */

void head(int fd, const char *p)
{
  static struct stat st;
  int copy = sizeof(buffer);
  int l;
  int ct = 0;


  if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode))
    copy = 1;
  while((l = read(fd, buffer, copy)) > 0) {
    outp = outbuf;
    for (p = buffer; l > 0; l--) {
      *outp++ = *p;
      if (outp == outbuf + 512) {
        write(1, outbuf, 512);
        outp = outbuf;
      }
      if (*p++ == '\n') {
        ct++;
        if (ct == lines) {
          write(1, outbuf, outp - outbuf);
          return;
        }
      }
    }
    write(1, outbuf, outp - outbuf);
  }
}

void fail(const char *p) {
  const char *e = strerror(errno);
  write(2, n, strlen(n));
  write(2, " cannot open '", 14);
  write(2, p, strlen(n));
  write(2, "' for reading: ", 15);
  write(2, e, strlen(e));
  write(2, "\n", 1);
  exit(1);
}

int main(int argc, char *argv[]) {
  const char *p;
  int fd;
  int err = 0;

  n = *argv++;
  p = *argv;

  if (p && *p == '-') {
    lines = atoi(p + 1);
    argv++;
  }
  if (!*argv)
    head(0, "stdin");
  else while((p = *argv++) != NULL) {
    fd = open(p, O_RDONLY);
    if (fd == -1) {
      fail(p);
      err = 1;
    } else {
      head(fd, p);
      close(fd);
    }
  }
  exit(err);
}
