/*
 * xargs.c
 *
 * Copyright 2000 Alistair Riddoch
 * ajr@ecs.soton.ac.uk
 *
 * This file may be distributed under the terms of the GNU General Public
 * License v2, or at your option any later version.
 */

/*
 * This is a small version of xargs for use in the ELKS project.
 * It is not fully functional, and may not be the most efficient
 * implementation for larger systems. It minimises memory usage and
 * code size.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 20
#define MAX_IARGS 10
#define MAX_CHARS 255

#define DEFAULT_CMD "/bin/echo"

int max_args = MAX_ARGS;
int max_chars = MAX_CHARS;	/* Not yet implemented */

char * progname;

/* Command to run if none specified */
char * default_cmd = DEFAULT_CMD;

/* New arguments for programs */
int nargc;
char * nargv[MAX_ARGS + MAX_IARGS + 2];

/*
 *
 * usage()
 *
 * Display usage information.
 *
 */

void usage(char ** argv)
{
	fprintf(stderr, "%s [-n max-args] [-s max-chars] [command [initial-arguments]]\n", argv[0]);
	exit(1);
}

/*
 *
 * do_args()
 *
 * Interpret command switches from the command line.
 *
 * int argc;		As passed to main().
 * char ** argv;	As passed to main().
 *
 * RETURN		Num of arguments interpreted.
 *
 */


int do_args(int argc, char ** argv)
{
	int i = 1, j, k;

	while(i < argc && argv[i][0] == '-') {
		k = 0;
		for(j = 1; j < strlen(argv[i]); j++) {
			switch (argv[i][j]) {
				case 'n':
					k++;
					if ((i + k) < argc) {
						max_args = atoi(argv[i+k]);
					}
					break;
				case 's':
					k++;
					if ((i + k) < argc) {
						max_chars = atoi(argv[i+k]);
					}
					break;
				default:
					usage(argv);
			}
		}
		i += k;
		i++;
	}
	if (max_args > MAX_ARGS) {
		max_args = MAX_ARGS;
	}
	if (max_chars > MAX_CHARS) {
		max_chars = MAX_CHARS;
	}
	return i;
}

/*
 *
 * build_cmd()
 *
 * Build the initial portion of the argv array to be used to run
 * commands from the command line arguments given for xargs.
 *
 * int argc;		Number of arguments left.
 * char ** argv;	Pointer to the first item in main()'s argv for us.
 *
 */

void build_cmd(int argc, char ** argv)
{
	int i;

	if (argc > (MAX_IARGS + MAX_ARGS - max_args)) {
		fprintf(stderr, "%s: Too many initial arguments.\n", progname);
		exit(1);
	}
	for(i = 0; i < argc; i++) {
		nargv[i] = argv[i];
	}
	nargc += argc;
}

#define BSIZE 64
#define out_of_mem() { perror("malloc"); exit(1); }

/*
 * next_token()
 *
 * Read standard in and get the next argument.
 * Returns NULL on end of file.
 *
 */

char * next_token(void)
{
	int size = BSIZE;
	int tail = 0;
	char * buf = malloc(BSIZE);

	if (buf == NULL) {
		out_of_mem();
	}
	for(;;) {
		int inp = fgetc(stdin);
		switch (inp) {
			case EOF:
				if (tail != 0) {
					buf[tail] = '\0';
				} else {
					free(buf);
					buf = NULL;
				}
				return buf;
				break;
			case ' ':
			case '\t':
			case '\n':
				if (tail != 0) {
					buf[tail] = '\0';
					return buf;
				}
				break;
			default:
				buf[tail++] = inp;
				break;
		}
		if (tail > size) {
			size += BSIZE;
			buf = realloc(buf, size);
			if (buf == NULL) {
				out_of_mem();
			}
		}
			
	}
}

/*
 *
 * run()
 *
 * Fork, exec and wait for the commant to complete.
 *
 * Parameters are as for execvp.
 *
 * We use vfork as this is a good saving under elks.
 *
 */

void run(char * argv0, char ** argv)
{
	int pid;
	
	pid = fork();
	switch (pid) {
		case -1:
			perror("fork");
			exit(1);
			break;
		case 0:
			break;
		default:
			wait(NULL);
			return;
	}
	execvp(argv0, argv);
	perror("argv0");
	exit(1);
}

int main(int argc, char ** argv)
{
	int num_args;
	int new_argc;
	char * tok = NULL;

	progname = argv[0];
	num_args = do_args(argc, argv);

	if (num_args >= argc) {
		build_cmd(1, &default_cmd);
	} else {
		build_cmd(argc - num_args, &argv[num_args]);
	}

	do {
		int i;

		new_argc = nargc;
		while(new_argc < (nargc + max_args) &&
		     (tok = next_token()) != NULL) {
			nargv[new_argc++] = tok;
		}
		nargv[new_argc] = NULL;
		if (nargc != new_argc) {
			run(nargv[0], nargv);
		}
		for(i = nargc; i < new_argc; i++) {
			free(nargv[i]);
		}
	} while (tok != NULL);

	exit(0);
}
