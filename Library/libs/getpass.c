/* getpass.c
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define EOF	(-1)

static int _getchar(int fd)
{
	static char ch;
	return (read(fd, &ch, 1) == 1) ? ch : EOF;
}

static char *_gets(int fd, char *buf, int len)
{
	int ch, i = 0;

	while (i < len) {
		if ((ch = _getchar(fd)) == EOF && i == 0)
			return NULL;
		if (ch == '\n' || ch == '\r')
			break;
		buf[i++] = ch;
	}
	buf[i] = 0;
	return buf;
}

char *getpass(char *prompt)
{
	static char result[128];
	struct termios t;
	tcflag_t ol;
	int tv;

	int fd = open("/dev/tty", O_RDWR);

	if (fd == -1)
		return NULL;

	/* display the prompt */
	write(fd, prompt, strlen(prompt));

	tv = tcgetattr(fd, &t);
	ol = t.c_lflag;
	t.c_lflag &= ~ECHO|ECHOE|ECHOK;
	if (tv == 0)
		tcsetattr(fd, TCSANOW, &t);
	/* read the input */
	if (_gets(fd, result, sizeof(result) - 1) == NULL)
		result[0] = 0;
	t.c_lflag = ol;
	if (tv == 0)
		tcsetattr(0, TCSANOW, &t);
	/* The newline isn't echoed as we have echo off, so we need to
	   output it as the end of the task */
	write(fd, "\n", 1);
	close(fd);
	return result;
}
