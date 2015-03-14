#include <stdio.h>
#include "syscall_name.h"

int main(int argc, const char* argv[])
{
	int i;

	for (i=0; i<NR_SYSCALL; i++)
	{
		printf("#define __syscall_%s %d\n", syscall_name[i], i);
		printf("#define __syscall_arg_%s %d\n", syscall_name[i], syscall_args[i]);
	}

	return 0;
}

