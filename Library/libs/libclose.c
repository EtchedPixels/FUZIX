#include <sys/library.h>
#include <fcntl.h>
#include <unistd.h>

int libclose(struct library *lib)
{
	close(lib->fd);
	lib->fd = -1;
	return 0;
}

