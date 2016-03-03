#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	if (af != AF_INET) {
		errno = EAFNOSUPPORT;
		return NULL;
	}
	if (size < 18) {
		errno = ENOSPC;
		return NULL;
	}
	/* This isn't strictly correct because it means we are not
	   re-entrant. Really we need to rework _itoa() to have a re-entrant
	   base form, then rework inet_ntoa to use inet_ntop FIXME */
	return strcpy(dst, _inet_ntoa(*(uint32_t *)src));
}
