/*      FUZIX specific includes and defines
 *
 */
#define FUZIX0_4	/* FUZIX 0.4 */
#define FUZIX

#define USES_TERMIOS
#define PROTO
#define STDARG

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>	/* Only for perror */
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <dirent.h>
#include <time.h>
#include <termcap.h>
