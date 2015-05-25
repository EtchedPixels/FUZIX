/*
 *	BSD has an API for this so we might as well provide a libc API
 *	using the same interface. Pity the API varies between systems on
 *	the type used for loadavg!
 */

#include <unistd.h>
#include <stdlib.h>

int getloadavg(unsigned int loadavg[], int nelem)
{
	static struct _uzisysinfoblk uts;
	uint8_t i;
	int bytes = _uname(&uts, sizeof(uts));

	if (bytes < sizeof(uts))
		return -1;
	if (nelem > 3)
		nelem = 3;
	for (i = 0; i < nelem; i++)
		loadavg[i] = (uts.loadavg[i] * 100) >> 8;
	return nelem;
}
