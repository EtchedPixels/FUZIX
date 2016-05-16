/*
 * Print an almanac of daily events.  Program reads today's date,
 * constructs a date string, and then examines the files
 * /usr/games/lib/almanac.birth, /usr/games/lib/almanac.death, and
 * /usr/games/lib/almanac.event for strings that contain the date string.
 * Originally written in the shell.
 *
 * By fb, 7/12/88.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

const char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char *filenames[] = {
	"/usr/games/lib/almanac.birth",
	"/usr/games/lib/almanac.death",
	"/usr/games/lib/almanac.event"
};

const char *slugs[] = {
	"BIRTHS:",
	"DEATHS:",
	"EVENTS:"
};

/* print an error message and exit */
void fatal(const char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

void findstring(const char *date, FILE * fileptr)
{
	char buffer[120];
	int length = strlen(date);

	while (fgets(buffer, 120, fileptr) != NULL) {
		if (strncmp(buffer, date, length) == 0)
			printf("%s", buffer + length);
		else
			continue;
	}
}

int main(int argc, char *argv[])
{
	time_t rawtime;
	struct tm *brokentime;
	FILE *fileptr;
	char buffer[20];
	int i;

	if (argc == 3) {
		if (strlen(argv[1]) < 3)
			fatal("Usage: almanac [Mon date]");
		*(argv[1] + 3) = '\0';
		sprintf(buffer, "%3s %s ", argv[1], argv[2]);
	} else if (argc == 1) {
		/* build the time string */
		rawtime = time(NULL);
		brokentime = localtime(&rawtime);
		sprintf(buffer, "%s %d ", months[brokentime->tm_mon],
			brokentime->tm_mday);
	} else
		fatal("Usage: almanac [Mon date]");

	/* open almanac text files; call string printing routine */
	for (i = 0; i < 3; i++) {
		if ((fileptr = fopen(filenames[i], "r")) == NULL)
			fatal("Cannot open almanac file.");

		printf("%s\n", slugs[i]);

		findstring(buffer, fileptr);

		fclose(fileptr);
	}
	exit(EXIT_SUCCESS);
}
