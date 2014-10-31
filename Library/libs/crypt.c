/* TEA based crypt(), version 0.0 <ndf@linux.mit.edu>
 * It looks like there are problems with key bits carrying through
 * to the encrypted data, and I want to get rid of that libc call..
 */  
#include <features.h>
#include <stdlib.h>
#include <memory.h>

char *crypt(char *key, char *salt) 
{
	
	    /* n is the number of rounds,
	     * delta is a golden # derivative,
	     * k is the key,
	     * v is the data to be encrypted.
	     */ 
	static char rkey[4 * sizeof(long)];
	unsigned long v[2], k[4], sum, delta = 0x9e3779b9UL, *p;
	unsigned char i, n;
	 
	/* Our constant string will be a string of zeros .. */ 
	memset(rkey, 0, sizeof(rkey));
	memcpy(rkey, salt, 2);

	i = 0;
	while (i < sizeof(rkey) - 2 && key[i]) {
		rkey[i + 2] = key[i];
		++i;
	}
	while (key[i]) {
		rkey[2] += key[i];
		++i;
	}
	memcpy(k, rkey, sizeof(k));
	v[0] = v[1] = sum = 0;
	for (i = 64; i != 0; --i) {
		sum += delta;
		v[0] +=
		    (v[1] << 4) + k[0] ^ v[1] + sum ^ (v[1] >> 5) + k[1];
		v[1] +=
		    (v[0] << 4) + k[2] ^ v[0] + sum ^ (v[0] >> 5) + k[3];
        }
	
	/* Now we need to unpack the bits and map it to "A-Za-z0-9./"
	 * for printing in /etc/passwd
	 */ 
	p = v;
	for (i = 2; i < 13; i++) {
		/* This unpacks the 6 bit data, each cluster into its own byte */ 
		if (i == 8) {
			v[0] |= v[1] >> 28;
			++p;
		}
		n = *p & 0x3F;
		*p >>= 6;
		
		/* Now we map to the proper chars */ 
		if (n < 12)
			n += '.';
		else if (n < 38)
			n += 'A' - 12;
		else
			n += 'a' - 38;
		rkey[i] = n;
	}
	rkey[13] = '\0';
	return rkey;
}

