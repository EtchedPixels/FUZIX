#include <termios.h>
#include <unistd.h>
#include <errno.h>

int tcflow(int fd, int action)
{
	struct termios term;
	cc_t c;

	switch (action) {
		case TCOOFF:
			return ioctl(fd, TIOCOSTOP, 0);
		case TCOON:
			return ioctl(fd, TIOCOSTART, 0);
		case TCIOFF:
		case TCION:
			if (tcgetattr(fd, &term) == -1)
				return -1;
			c = term.c_cc[action == TCIOFF ? VSTOP : VSTART];
			if (c != _POSIX_VDISABLE && (write(fd, &c, sizeof(c))) == -1)
				return -1;
			return 0;
		default:
			errno = EINVAL;
			return -1;
	}
}
