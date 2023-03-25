#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define AF_INET		1
#define SOCK_STREAM	3
#define IPPROTO_TCP	6

struct sockaddr_in addr;
struct sockaddr_in laddr;

static int writes(int fd, const char *p)
{
	return write(fd, p, strlen(p));
}

int main(int argc, char *argv[])
{
	int fd, fd2 = -1;
	int r;
	unsigned long t = 0;
	static char buf[512];
	static char line[81];
	char *lp;
	char *p;
	char *fn;
	int state = 0;

	if (argc != 2) {
		writes(2, argv[0]);
		write(2, ": url\n", 6);
		exit(1);
	}

	if (strncmp(argv[1], "http://", 7)) {
		writes(2, argv[0]);
		writes(2, ": only http:// supported.\n");
		exit(1);
	}

	p = strchr(argv[1] + 7, '/');
	if (p == NULL) {
		writes(2, argv[0]);
		writes(2, ": no path element.\n");
		exit(1);
	}
	*p++ = 0;
	fn = p;

	lp = strchr(argv[1] + 7, ':');
	if (lp == NULL)
		addr.sin_port = htons(80);
	else {
		*lp = 0;
		addr.sin_port = htons(atoi(lp + 1));
	}

	if (inet_pton(AF_INET, argv[1] + 7, &addr.sin_addr.s_addr) <= 0) {
		writes(2, argv[1] + 7);
		writes(2, ": unknown address.\n");
		exit(1);
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("af_inet sock_stream 0");
		exit(1);
	}
	laddr.sin_family = AF_INET;
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	laddr.sin_port = htons(8000);

	if (bind(fd, (struct sockaddr *) &laddr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}

	addr.sin_family = AF_INET;
	printf("%lx\n", addr.sin_addr.s_addr);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("connect");
		exit(1);
	}
	if (writes(fd, "GET /") == -1 ||
	    writes(fd, fn) == -1 ||
	    writes(fd, " HTTP/1.1\r\nHost: ") == -1 ||
	    writes(fd, argv[1] + 7) == -1 ||
	    writes(fd, "\r\n\r\n") == -1) {
		perror("write");
		exit(1);
	}

	p = strrchr(fn, '/');
	if (p)
	        fn = p + 1;

	lp = line;

	while ((r = read(fd, buf, 512)) > 0) {
		if (r < 0) {
			perror("read");
			exit(1);
		}
		p = buf;
		switch (state) {
		case 0:
		case 1:
			while (p < buf + r) {
				if (*p != '\n') {
					if (*p != '\r' && lp != line + 80)
						*lp++ = *p;
					p++;
					continue;
				}
				/* We have a line */
				if (lp != line) {
					*lp = 0;
					write(1, "|", 1);
					writes(1, line);
					write(1, "\n", 1);
					if (state == 0) {
						lp = strchr(line, ' ');
						if (lp == NULL
						    || atoi(lp + 1) != 200)
							exit(1);
					}
					state = 1;
					lp = line;
					p++;
					continue;
				} else {
					p++;
					/* nl/nl end of headers - begin xfer */
					fd2 =
					    open(fn,
						 O_WRONLY | O_TRUNC |
						 O_CREAT, 0700);
					if (fd2 == -1) {
						perror("output");
						exit(1);
					}
					r -= (p - buf);
					if (r && write(fd2, p, r) != r) {
						perror("write");
						exit(1);
					}
					state = 2;
					break;
				}
			}
			break;
		case 2:
			if (write(fd2, buf, r) != r) {
				perror("write");
				exit(1);
			}
			t += r;
			printf("%ld\r", t);
			fflush(stdout);
			break;
		}
	}
	printf("\nRead %ld bytes\n", t);
	close(fd);
	close(fd2);
	return 0;
}
