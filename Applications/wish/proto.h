/* Prototypes for Wish. This file contains the K&R and Ansi prototypes for
 * all the non-static routines in the shell, plus prototypes for all the
 * external routines that the shell uses. Expect to comment some of these
 * out depending upon what is already defined in your system's header files.
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#ifdef PROTO
# define	P(s) s
#else
# define	P(s) ()
#endif


/* Trying to use the intuitive prototypes below AND
 * allow the use of stdargs or varargs is basically
 * impossible.
 *
 */
#ifndef NO_PRINTS_DEFN
void fprints P((int fd, CONST char *fmt, ...));
void sprints P((char *out, CONST char *fmt, ...));
void prints P((CONST char *fmt, ...));
#endif

void mprint P((uchar *line , int nocr ));

/* alias.c */
struct val *checkalias P((char *aname ));
bool getaliasline P((uchar *line , int *nosave ));
int alias P((int argc , char *argv []));
int unalias P((int argc , char *argv []));

/* bind.c */
int Bind P((int argc , uchar *argv []));
int unbind P((int argc , uchar *argv []));
void initbind P((void ));
int getcomcmd P((void ));

/* builtin.c */
int builtin P((int argc , char *argv [], int *rtnpid ));

/* clebuf.c */
void flushbuf P((void ));
void addbuf P((char *str ));
void mputc P((int b ));

/* clex.c */
int compare P((CONST void *a , CONST void *b ));
void addcarray P((char *word , struct candidate *prev , int mode , bool malc ));
void complete P((char *line , int *pos , bool how ));

/* comlined.c */
int Show P((uchar *line , int pos , int let , int flag ));
int prevword P((uchar *line , int *p , int flag ));
bool getuline P((uchar *line , int *nosave ));

/* exec.c */
int invoke P((int argc , char *argv [], struct rdrct newfd [], int how , int anydups ));

/* file.c */
bool fileopen P((char *filename , int fd ));
void fileclose P((void ));
bool getfileline P((uchar *line , int *nosave ));
int source P((int argc , char *argv []));

/* hist.c */
int savehist P((char *line , bool andadd ));
void loadhist P((char *line , int histnum ));
int history P((int argc , char *argv []));
char *gethist P((char *event ));

/* job.c */
int addjob P((int pid , char *name , int isbg ));
int joblist P((int argc , char *argv []));
int Kill P((int argc , char *argv []));
int bg P((int argc , char *argv []));
int fg P((int argc , char *argv []));

/* posixjob.c */
void waitfor P((int pid ));
SIGTYPE stopjob P((int a));

/* main.c */
void prprompt P((void ));
void leave_shell P((int how ));
void doline P((int isalias ));
int main P((int argc , char *argv []));

/* malloc.c */
char *Malloc P((unsigned int size , char *mesg ));
void initmall P((void ));
void *myMalloc P((unsigned int size ));
void myFree P((void *ptr ));

/* meta.c */
void tilde P((char *word , char *dir ));
char *expline P((struct candidate *list ));
bool meta_1 P((char *old , bool mustmalc ));
void meta_2 P((void ));

/* parse.c */
int command P((int *waitpid , int makepipe , int *pipefdp , int anydups ));

/* signal.c */
void catchsig P((void ));
void dflsig P((void ));

/* term.c */
void getstty P((void ));
void setcbreak P((void ));
void setcooked P((void ));
void getstty P((void ));
void setcbreak P((void ));
void setcooked P((void ));
void getstty P((void ));
void setcbreak P((void ));
void setcooked P((void ));
void terminal P((void ));

/* val.c */
void appendval P((struct vallist *l , struct val *v ));
struct val *searchval P((struct vallist *l , char *name , int mode , bool sub ));
void saveval P((struct vallist *l , struct val *v ));
struct val *pullval P((struct vallist *l ));
void setval P((char *name , char *val , struct vallist *l ));

/* var.c */
char *EVget P((char *name ));
bool EVinit P((void ));
bool EVupdate P((void ));
int export P((int argc , char *argv []));
int shift P((int argc , char *argv []));
int unset P((int argc , char *argv []));
int set P((int argc , char *argv []));



/* The following are defined so that gcc -Wall won't complain about
 * missing prototypes - you may need to alter or comment them out
 * for your own machine. Some of these were borrowed from Minix.
 */


#ifdef UNUSED
/* Ansi C routines */

int atoi P((CONST char *nptr));
void free P((void *ptr));
void *malloc P((size_t size));
void *memcpy P((void *s1, CONST void *s2, size_t n));
void *memset P((void *s, int c, size_t n));
void qsort P((void *base, size_t nmemb, size_t size,
	int (*compare)(CONST void *a, CONST void *b)));
void *realloc P((void *ptr, size_t size));
char *strcat P((char *s1, CONST char *s2));
char *strchr P((CONST char *s, int c));
int strcmp P((CONST char *s1, CONST char *s2));
char *strcpy P((char *s1, CONST char *s2));
size_t strlen P((CONST char *s));
int strncmp P((CONST char *s1, CONST char *s2, size_t n));
char *strncpy P((char *s1, CONST char *s2, size_t n));
char *strpbrk P((CONST char *s1, CONST char *s2));
char *strrchr P((CONST char *s, int c));


/* POSIX routines */

int chdir P((CONST char *path));
int closedir P((DIR *dirp));
int close P((int fd));
int dup P((int fd));
int dup2 P((int fd, int fd2));
int execvp P((CONST char *file, char *argv[]));
void exit P((int status));
pid_t fork P((void));
char *getcwd P((char *buf, int size));
pid_t getpid P((void));
uid_t getuid P((void));
struct passwd *getpwent P((void));
struct passwd *getpwnam P((CONST char *name));
struct passwd *getpwuid P((int uid));
void endpwent P((void));
int kill P((pid_t pid, int sig));
struct tm *localtime P((CONST time_t *timer));
off_t lseek P((int fd, off_t offset, int whence));
int open P((CONST char *path, int oflag, mode_t modes));
void perror P((CONST char *s));
int pipe P((int fildes[2]));
int read P((int fd, char *buf, unsigned int n));
int setpgid P((int pid, int pgid));
int getpgrp P((int pid));
SIGTYPE (*signal)();
int stat P((CONST char *filename, struct stat *s));
int tcgetattr P((int fd, struct termios *tptr));
int tcsetattr P((int fd, int opt_actions, struct termios *tptr));
int tcsetpgrp P((int fd, int pgrp));
time_t time P((time_t *timeptr));
pid_t wait P((int *stat_loc));
pid_t waitpid P((pid_t pid, int *stat_loc, int options));
int write P((int fd, CONST char *buf, unsigned int n));
DIR *opendir P((CONST char *dirname));
int closedir P((DIR *dirp));
#ifdef USES_DIRECT
struct direct *readdir P((DIR *dirp));
#else
struct dirent *readdir P((DIR *dirp));
#endif


/* Other routines */

char *getcwd P((char *buf));
int ioctl P((int fd, int request, ... ));
int setpgrp P((int pid, int pgid));
int tgetent P((char *bp, CONST char *name));
int tgetflag P((CONST char *id));
int tgetnum P((CONST char *id));
char *tgetstr P((CONST char *id, char **area));
int wait3 P((union wait *status, int options, struct rusage *rusage));
void bzero P((char *b1, int length));
void bcopy P((CONST char *b1, char *b2, int length));

#endif	/* UNUSED */

#undef P
