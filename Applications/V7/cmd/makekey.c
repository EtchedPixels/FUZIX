/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * You send it 10 bytes.
 * It sends you 13 bytes.
 * The transformation is expensive to perform
 * (a significant part of a second).
 */

#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	char key[8];
	char salt[2];
	
	read(0, key, 8);
	read(0, salt, 2);
	write(1, crypt(key, salt), 13);
	return(0);
}
