#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/* Keep this as tight as possible. It's the routine people should be using
   for address printing and it's not got all the legacy crap in it so can
   be small and clean */

/* Yes the error path for this is slower than it needs to be. If you are
   optimzing for the error path you are doing something wrong. Instead it's
   optimized for size and the normal path */

static const char *quad(const char *p, uint8_t *r)
{
	uint16_t val = 0;
	uint8_t valid = 0;

	if (p == NULL)
		return NULL;

	while(*p >= '0' && *p <= '9') {
		val = (val * 10) + *p++ - '0';
		valid++;
		if (valid > 3)
			return NULL;
	}
	if (val > 255)
		return NULL;
	*r = val;
	if (*p == '.')
		return p + 1;
	if (*p == 0)
		return p;
	return NULL;
}
		
int inet_pton(int af, const char *src, void *dst)
{
	uint8_t *p = dst;

	if (af != AF_INET) {
		errno = EAFNOSUPPORT;
		return 0;
	}

	/* Unlike the legacy interfaces this one requires decimal and four
	   dotted quads */
	src = quad(src, p++);
	src = quad(src, p++);
	src = quad(src, p++);
	src = quad(src, p);
	if (src == NULL || *src)
		return 0;
	return 1;
}
