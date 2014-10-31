/*
 * utsname.c for UZIX
 * by A&L Software 1999
 *
 *
 * FIXME: rewrite for fuzix string packing
 */

#include <utsname.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* FIXME */
int uname(struct utsname *__utsbuf)
{

#if 0
	struct s_kdata kdata;
	int i;
	getfsys(GI_KDAT, &kdata);
	strcpy(__utsbuf->sysname, kdata.k_name);
	strcpy(__utsbuf->nodename, kdata.k_name);
	for (i = 0; i < strlen(__utsbuf->nodename); i++)
		__utsbuf->nodename[i] = tolower(__utsbuf->nodename[i]);
	strcpy(__utsbuf->release, kdata.k_release);
	strcpy(__utsbuf->version, kdata.k_version);
	strcpy(__utsbuf->machine, kdata.k_machine);
	strcpy(__utsbuf->domainname, "(localhost)");
	return 0;

#endif				/*  */
}
