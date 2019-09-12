#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ERRORS	57
const char *err[57] = {
  "Success",
  "Operation not permitted",
  "No such file or directory",
  "No such process",
  "Interrupted system call",
  "I/O error",
  "No such device or address",
  "Arg list too long",
  "Exec format error",
  "Bad file number",
  "No child processes",
  "Try again",
  "Out of memory",
  "Permission denied",
  "Bad address",
  "Block device required",
  "Device or resource busy",
  "File exists",
  "Cross-device link",
  "No such device",
  "Not a directory",
  "Is a directory",
  "Invalid argument",
  "File table overflow",
  "Too many open files",
  "Not a typewriter",
  "Text file busy",
  "File too large",
  "No space left on device",
  "Illegal seek",
  "Read-only file system",
  "Too many links",
  "Broken pipe",
  "Math argument out of domain of efunc",
  "Math result not representable",
  "Lock table full",
  "Directory is not empty",
  "File name too long",
  "Address family not supported",
  "Operation already in progress",
  "Address not available",
  "Invalid system call number",
  "Protocol family not supported",
  "Operation not supported on transport endpoint",
  "Connection reset by peer",
  "Network is down",
  "Message too long",
  "Connection timed out",
  "Connection refused",
  "No route to host",
  "Host is down",
  "Network is unreachable",
  "Transport endpoint is not connected",
  "Operation is in progress",
  "Cannot send after transport endpoint shutdown",
  "Socket is already connected",
  "No destination address specified"
};

static uint8_t buf[16384];

int main(int argc, char *argv[])
{
  int base = 2 * ERRORS + 2;
  uint8_t *bp = buf + 2;
  int swizzle = 0;
  int i;
  
  /* FIXME we can only swizzle to big-endian here */
  if (argc > 1 && strcmp(argv[1], "-X") == 0)
    swizzle = 1;

  if (swizzle) {
    buf[1] = ERRORS;
    buf[0] = 0;
  } else {
    buf[0] = ERRORS;
    buf[1] = 0;
  }

  for (i = 0; i < ERRORS; i++) {
    int len = strlen(err[i]) + 1;
    if (swizzle) {
      *bp++ = base >> 8;
      *bp++ = base & 0xFF;
    } else {
      *bp++ = base & 0xFF;
      *bp++ = base >> 8;
    }
    memcpy(buf + base, err[i], len);
    base += len;
  }
  write(1, buf, base);
  return 0;
}
