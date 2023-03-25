/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied and defloated for FUZIX */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define	MAX	100

char types[10];
int right[MAX];
int left[MAX];
int rights;
int wrongs;
time_t stvec;
time_t etvec;
time_t dtvec;


void getline(char *s)
{
	register char *rs;

	rs = s;

	fflush(stdout);
	while ((*rs = getchar()) == ' ');
	while (*rs != '\n')
		if (*rs == 0)
			exit(0);
		else if (rs >= &s[99]) {
			while ((*rs = getchar()) != '\n')
				if (*rs == '\0')
					exit(0);
		} else
			*++rs = getchar();
	while (*--rs == ' ')
		*rs = '\n';
}

int getnum(char *s)
{
	int a;
	char c;

	a = 0;
	while ((c = *s++) >= '0' && c <= '9') {
		a = a * 10 + c - '0';
	}
	return (a);
}


int random(int range)
{
	return (rand() % range);
}

int skrand(int range)
{
	int temp;
	temp = random(range) + random(range);
	if (temp > range - 1)
		temp = 2 * range - 1 - temp;
	return (temp);
}

void score(void)
{
	long l;
	time(&etvec);

	printf("\n\nRights %d; Wrongs %d; Score %d%%\n", rights, wrongs,
	       (rights * 100) / (rights + wrongs));

	if (rights == 0)
		return;

	l = etvec - stvec;

	printf("Total time %ld seconds; %d.%2d seconds per problem\n\n\n",
	       l, l / rights, ((l * 100) / rights)%100);

	sleep(3);
	time(&dtvec);
	stvec += dtvec - etvec;
}

void delete(int sig)
{
	if (rights + wrongs == 0) {
		printf("\n");
		exit(0);
	}
	score();
	exit(0);
}

int main(int argc, char *argv[])
{
	int range, k, dif, l;
	char line[100];
	int ans, pans, i, j, t;

	signal(SIGINT, delete);

	range = 11;
	dif = 0;
	while (argc > 1) {
		switch (*argv[1]) {
		case '+':
		case '-':
		case 'x':
		case '/':
			while ((types[dif] = argv[1][dif]) != 0)
				dif++;
			break;

		default:
			range = getnum(argv[1]) + 1;
		}
		argv++;
		argc--;
	}
	if (range > MAX) {
		printf("Range is too large.\n");
		exit(0);
	}

	if (dif == 0) {
		types[0] = '+';
		types[1] = '-';
		dif = 2;
	}

	for (i = 0; i < range; i++) {
		left[i] = right[i] = i;
	}
	time(&stvec);
	k = stvec;
	srand(k);
	k = 0;
	l = 0;
	goto start;

      loop:
	if (++k % 20 == 0)
		score();

      start:
	i = skrand(range);
	j = skrand(range);
	if (dif > 1)
		l = random(dif);

	switch (types[l]) {
	case '+':
	default:
		ans = left[i] + right[j];
		printf("%d + %d =   ", left[i], right[j]);
		break;

	case '-':
		t = left[i] + right[j];
		ans = left[i];
		printf("%d - %d =   ", t, right[j]);
		break;

	case 'x':
		ans = left[i] * right[j];
		printf("%d x %d =   ", left[i], right[j]);
		break;

	case '/':
		while (right[j] == 0)
			j = random(range);
		t = left[i] * right[j] + random(right[j]);
		ans = left[i];
		printf("%d / %d =   ", t, right[j]);
		break;
	}


      loop1:
	getline(line);
	dtvec += etvec - stvec;
	if (line[0] == '\n')
		goto loop1;
	pans = getnum(line);
	if (pans == ans) {
		printf("Right!\n");
		rights++;
		goto loop;
	} else {
		printf("What?\n");
		wrongs++;
		if (range >= MAX)
			goto loop1;
		left[range] = left[i];
		right[range++] = right[j];
		goto loop1;
	}
}
