/* adjust.c - handle the # command */

#include <string.h>

#include "s.h"

#define LENGTH 78

extern void b_getcur(), b_getmark(), b_gets(), b_delete(), b_setcur();
extern int b_insert();

void adjust(n)
int n;	/* line length */
{
	char *b, *p;
	int start, end, temp, remaining, put_next, get_next, pos, len;
	char buf[1000];

	if (n == 0)
		n = LENGTH;
	b_getcur(&start, &pos);
	end = 0;
	b_getmark(&end, &pos);
	if (end == 0) {
		UNKNOWN;
		return;
	}
	if (start > end) {
		temp = start;
		start = end;
		end = temp;
	}
	*buf = '\0';
	put_next = get_next = start;
	for (remaining = end - start + 1; remaining > 0; ) {
		while (remaining > 0 && (len = strlen(buf)) < n) {
			if (len > 0 && buf[len-1] != ' ') {
				/* add spacer */
				buf[len++] = ' ';
				buf[len] = '\0';
			}
			b_gets(get_next, buf + len);
			b_delete(get_next, get_next);
			--remaining;
		}
		while ((int)strlen(buf) >= n) {
			for (b = buf + n; b > buf && *b != ' '; --b)
				;
			if (b == buf) {
				b_insert(put_next++, buf);
				++get_next;
				break;
			}
			*b = '\0';
			b_insert(put_next++, buf);
			++get_next;
			for (p = buf, ++b; (*p++ = *b++) != '\0'; )
				;
		}
	}
	if (*buf != '\0')
		b_insert(put_next, buf);
	b_setcur(start, 0);
}
