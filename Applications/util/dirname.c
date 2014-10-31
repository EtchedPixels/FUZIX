#include <string.h>
#include <unistd.h>
#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

void strip_trailing_slashes(char *path)
{
    int last;

    last = strlen(path) - 1;
    while (last > 0 && path[last] == '/')
	path[last--] = '\0';
}


int main(int argc, char *argv[])
{
    char *line;

    if (argc == 2) {
	strip_trailing_slashes(argv[1]);
	line = rindex(argv[1], '/');
	if (line == NULL) {
	    line = ".";
	} else {
	    while (line > argv[1] && *line == '/')
		--line;
	    line[1] = 0;
	    line = argv[1];
	}

	write(STDOUT_FILENO, line, strlen(line));
	write(STDOUT_FILENO, "\n", 1);
    }
    return 0;
}
