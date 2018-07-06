/*	interact.c	1.8	83/07/27	*/
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include "command.h"
#include "cpm.h"

#define errinp { printf("??\n"); break; }

jmp_buf env;
int firsttime = 0;

void interact(void)
{

	int i;
	char cmd[80], *rest;

	for (;;) {
		if (firsttime++ == 0) {
			signal(SIGINT, intrpt);
			(void) setjmp(env);
		}
		cmdinp(cmd, 80);
		rest = strchr(cmd, ' ');
		if (rest)
			*rest++ = '\0';
		i = chkcmd(cmd);
#ifdef DEBUG
		printf("command: %s, ind: %d\n", cmd, i);
#endif

		switch (i) {
		default:
			errinp;
			break;
		case CMD_DIR:
		case CMD_LS:
			dispdir();
			break;

		case CMD_RENAME:
			Rename(rest);
			break;

		case CMD_OCOPY:
			copyc(rest, 0);
			break;

		case CMD_ICOPY:
			pip(rest, 0);
			break;

		case CMD_DELETE:
		case CMD_ERASE:
			delete(rest);
			break;

		case CMD_EXIT:
		case CMD_LOGOUT:
			return;

		case CMD_TYPE:
			copy(rest, NULL, 0);
			break;

		case CMD_HELP:
			help();
			break;

		case CMD_OCCOPY:
			copyc(rest, 1);
			break;

		case CMD_ICCOPY:
			pip(rest, 1);
			break;

		case CMD_DUMP:
			dump(rest);
			break;

		}
	}
}

/*
 * handle interrupts (in interactive mode only),
 * just (long)jump back to command input mode
 */

void intrpt(int sig)
{
	signal(sig, intrpt);
	firsttime = 0;
	printf("\n");
	longjmp(env, 0);
}
