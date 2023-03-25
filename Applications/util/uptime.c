#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscalls.h>
#include <time.h>
#include <utmp.h>

static int count_users(void)
{
	int ct = 0;
	struct utmp *entry;
	setutent();
	while((entry = getutent()) != NULL)
		if (entry->ut_type == USER_PROCESS)
			ct++;
	endutent();
	return ct;
}

static void printload(int i)
{
	unsigned int n = i;
	printf(" %d.%02d", n/100, n%100);
}

int main(int argc, char *argv[])
{
	static struct timespec res;
	static unsigned int loadavg[3];
	time_t t;
	uint32_t days, hours, mins;
	struct tm *tm;
	int u;

	/* Get the clocks we want */
	clock_gettime(CLOCK_MONOTONIC, &res);
        time(&t);
	tm = localtime(&t);
	printf(" %02d:%02d:%02d up ",
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	mins = res.tv_sec / 60;
	hours = mins / 60;
	days = hours / 24;

	mins %= 60;
	hours %= 24;

	if (days == 1)
		printf("1 day, ");
	else if (days > 1)
		printf("%d days, ", (unsigned int)days);
	printf("%02d:%02d, ", (unsigned int)hours, (unsigned int)mins); 

	u = count_users();
	printf("%d user%s", u, u != 1 ? "s" : "");

	if (getloadavg(loadavg, 3) == 3) {
		printf(", load average:");
		printload(loadavg[0]);
		printload(loadavg[1]);
		printload(loadavg[2]);
	}
	printf("\n");
	return 0;
}
