/* getpass.c
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

#define EOF	(-1)

static int _getchar(void)
{
	static char ch;
	return (read(0, &ch, 1) == 1) ? ch : EOF;
}

static char *_gets(char *buf, int len)
{
	int ch, i = 0;

	while (i < len) {
		if ((ch = _getchar()) == EOF && i == 0)
			return NULL;
		if (ch == '\n' || ch == '\r')
			break;
		buf[i++] = ch;
	}
	buf[i] = 0;
	return buf;
}

/* FIXME: should use /dev/tty interface eventually */
char *getpass(char *prompt)
{
	static char result[128];
	struct termios t;
	tcflag_t ol;
	int tv;

	/* display the prompt */
	write(2, prompt, strlen(prompt));

	tv = tcgetattr(0, &t);
	ol = t.c_lflag;
	t.c_lflag &= ~ECHO|ECHOE|ECHOK;
	if (tv == 0)
		tcsetattr(0, TCSANOW, &t);
	/* read the input */
	if (_gets(result, sizeof(result) - 1) == NULL)
		result[0] = 0;
	t.c_lflag = ol;
	if (tv == 0)
		tcsetattr(0, TCSANOW, &t);
	return result;
}
