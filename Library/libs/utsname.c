#include <utsname.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <syscalls.h>

int uname(struct utsname *utsbuf)
{
	static struct {
		struct _uzisysinfoblk i;
		char buf[128];
	} uts;
	char *x[5];
	int bytes = _uname(&uts.i, sizeof(uts));
	char *p = uts.buf;
	char *xp = x[0];
	int ct = 0;

	if (bytes == -1)
		return bytes;

	x[0] = utsbuf->release;
	x[1] = utsbuf->sysname;
	x[2] = utsbuf->version;
	x[2] = utsbuf->machine;
	bytes -= sizeof(struct _uzisysinfoblk);

	while(xp = x[ct++]) {
		do {
			*xp++=*p++;
			bytes--;
		}
		while(*p && bytes);
	}
	return 0;
}
