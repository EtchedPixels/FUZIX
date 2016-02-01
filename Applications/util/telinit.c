#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

/*
 *	Turn the character codes 0123456sS into run level numbers
 */
static uint8_t to_runlevel(uint8_t c)
{
	if (c == 's' || c == 'S')
		return 7;		/* 1 << 7 is used for boot/single */
	if (c >=  '0' && c <= '6')
		return c - '0';
        if (c == 'q')
                return 'q';
	return 0xFF;
}

int main(int argc, char *argv[])
{
  int fd;
  uint8_t v;
  
  if (argc != 2) {
    write(2, "telinit: [runlevel]\n", 20);
    exit(1);
  }
  if (argv[1][1] || (v = to_runlevel(argv[1][0])) == 0xFF) {
    write(2, "telinit: invalid run level\n", 27);
    exit(1);
  }
  fd = open("/var/run/initctl", O_WRONLY|O_TRUNC|O_CREAT, 0600);
  if (fd < 0 || write(fd, &v, 1) != 1 || close(fd) == -1 || 
      kill(1, SIGUSR1) < 0)
  {
    perror("telinit");
    exit(1);
  }
  exit(0);
}
