#include <stdio.h>
#include <unistd.h>
#include <syscalls.h>
#include <time.h>
#include <utmp.h>

static int count_users(void)
{
	int ct = 0;
	struct utmp *entry;
	setutent();
	while(entry = getutent())
		if (entry->ut_type == USER_PROCESS)
			ct++;
	endutent();
	return ct;
}

int main(int argc, char *argv[])
{
	static struct {
		struct _uzisysinfoblk i;
		char buf[128];
	} uts;
	static struct timespec res;
	int bytes = _uname(&uts.i, sizeof(uts));
	time_t t;
	uint32_t days, hours, mins;
	struct tm *tm;
	int u;

	/* Get the clocks we want */
	clock_gettime(CLOCK_MONOTONIC, &res);
        time(&t);
	tm = localtime(&t);
	printf(" %2d:%02d:%02d up ",
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	mins = res.tv_sec / 60;
	hours = mins / 60;
	days = hours / 24;

	mins %= 60;
	hours %= 24;

	if (days == 1)
		printf("1 day, ");
	else if (days > 1)
		printf("%d days, ", days);
	printf("%02d:%02d, ", hours, mins); 

	u = count_users();
	printf("%d user%s, ", u, u != 1 ? "s" : "");

	printf("load average: %d %d %d\n",
	  uts.i.loadavg[0],
	  uts.i.loadavg[1],
	  uts.i.loadavg[2]);
}

