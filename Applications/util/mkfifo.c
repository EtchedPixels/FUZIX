#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int main (int argc, char *argv[])
{
	unsigned short newmode;
	int i,er=0;
	
	newmode = 0666 & ~umask(0);
	for(i=1;i<argc;i++)
	{
		if (mkfifo (argv[i],newmode)) {
			write(STDERR_FILENO,"mkfifo: cannot make fifo ",25);
			write(STDERR_FILENO,argv[i],strlen(argv[i]));
			write(STDERR_FILENO,"\n",1);
			er|=1;
		}
	}
	return er;
}
