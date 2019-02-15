/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */


#include	"defs.h"
#include	"sym.h"

const char version[] =
    "\nVERSION sys137	DATE 1978 Nov 6 14:29:22\n";

/* error messages */
const char badopt[] = "bad option(s)";
const char mailmsg[] = "you have mail\n";
const char nospace[] = "no space";
const char synmsg[] = "syntax error";

const char badnum[] = "bad number";
const char badparam[] = "parameter not set";
const char badsub[] = "bad substitution";
const char badcreate[] = "cannot create";
const char illegal[] = "illegal io";
const char restricted[] = "restricted";
const char piperr[] = "cannot make pipe";
const char badopen[] = "cannot open";
const char coredump[] = " - core dumped";
const char arglist[] = "arg list too long";
const char txtbsy[] = "text busy";
const char toobig[] = "too big";
const char badexec[] = "cannot execute";
const char notfound[] = "not found";
const char badfile[] = "bad file number";
const char badshift[] = "cannot shift";
const char baddir[] = "bad directory";
const char badtrap[] = "bad trap";
const char wtfailed[] = "is read only";
const char notid[] = "is not an identifier";

/* built in names */
const char pathname[] = "PATH";
const char homename[] = "HOME";
const char mailname[] = "MAIL";
const char fngname[] = "FILEMATCH";
const char ifsname[] = "IFS";
const char ps1name[] = "PS1";
const char ps2name[] = "PS2";

/* string constants */
const char nullstr[] = "";
const char sptbnl[] = " \t\n";
const char defpath[] = ":/bin:/usr/bin";
const char colon[] = ": ";
const char minus[] = "-";
const char endoffile[] = "end of file";
const char unexpected[] = " unexpected";
const char atline[] = " at line ";
const char devnull[] = "/dev/null";
const char execpmsg[] = "+ ";
const char readmsg[] = "> ";
const char stdprompt[] = "$ ";
const char supprompt[] = "# ";
const char profile[] = ".profile";


/* tables */
SYSTAB reserved = {
	{"in", INSYM},
	{"esac", ESSYM},
	{"case", CASYM},
	{"for", FORSYM},
	{"done", ODSYM},
	{"if", IFSYM},
	{"while", WHSYM},
	{"do", DOSYM},
	{"then", THSYM},
	{"else", ELSYM},
	{"elif", EFSYM},
	{"fi", FISYM},
	{"until", UNSYM},
	{"{", BRSYM},
	{"}", KTSYM},
	{0, 0},
};

const char export[] = "export";
const char readonly[] = "readonly";
SYSTAB commands = {
	{"cd", SYSCD},
	{"read", SYSREAD},
/*
		{"[",		SYSTST},
*/
	{"set", SYSSET},
	{":", SYSNULL},
	{"trap", SYSTRAP},
	{"login", SYSLOGIN},
	{"wait", SYSWAIT},
	{"eval", SYSEVAL},
	{".", SYSDOT},
	{"newgrp", SYSLOGIN},
	{readonly, SYSRDONLY},
	{export, SYSXPORT},
	{"chdir", SYSCD},
	{"break", SYSBREAK},
	{"continue", SYSCONT},
	{"shift", SYSSHFT},
	{"exit", SYSEXIT},
	{"exec", SYSEXEC},
	{"times", SYSTIMES},
	{"umask", SYSUMASK},
	{0, 0},
};
