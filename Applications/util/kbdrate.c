/*
  Small utility to set the keyboard repeat/delay rates
*/

#include <stdlib.h>
#include <string.h>
#include <syscalls.h>
#include <sys/kd.h>
#include <fcntl.h>

void printe(char *m)
{
	write(2, m, strlen(m));
	write(2, "\n", 1);
	exit(1);
}

int main(int argc, char **argv)
{
	int fd;
	struct key_repeat k;
	int ret;

	if(argc != 3)
		printe( "usage: kbdrate rate delay");

	fd=open("/dev/tty", O_RDONLY);
	if(!fd)
		printe("Cannot open tty dev");
	
	k.continual = atoi(argv[1]);
	k.first = atoi(argv[2]);

	/* limit horribleness */
	if(k.continual == 0)
		k.continual = 1;
	if(k.first == 0)
		k.first = 1;

	ret = ioctl(fd, KBRATE, &k);
	if(ret) {
		printe("failed ioctl");
	}

	close(fd);
	exit(0);
}
