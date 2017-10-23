#include <stdio.h>
#include <unistd.h>
#include <syscalls.h>
#include <time.h>

int main(int argc, char *argv[])
{
	static struct {
		struct _uzisysinfoblk i;
		char buf[128];
	} uts;
	int bytes = _uname(&uts.i, sizeof(uts));
	printf("         total         used         free\n");
	printf("Mem:     %5d        %5d        %5d\n",
		uts.i.memk, uts.i.usedk, uts.i.memk - uts.i.usedk);
	printf("Swap:    %5d        %5d        %5d\n",
		uts.i.swapk, uts.i.swapusedk, uts.i.swapk - uts.i.swapusedk);
	return 0;
}

