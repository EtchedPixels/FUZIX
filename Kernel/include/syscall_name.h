#define NR_SYSCALL 80

#define VARARGS 	-1

const char *syscall_name[NR_SYSCALL] = {
	"_exit",
	"open",
	"close",
	"rename",
	"mknod",
	"link",
	"unlink",
	"read",
	"write",
	"_lseek",
	"chdir",
	"sync",
	"access",
	"chmod",
	"chown",
	"_stat",
	"_fstat",
	"dup",
	"getpid",
	"getppid",
	"getuid",
	"umask",
	"_statfs",
	"execve",
	"_getdirent",
	"setuid",
	"setgid",
	"_time",
	"_stime",
	"ioctl",
	"brk",
	"sbrk",
	"_fork",
	"mount",
	"_umount",
	"signal",
	"dup2",
	"_pause",
	"_alarm",
	"kill",
	"pipe",
	"getgid",
	"times",
	"utime",
	"geteuid",
	"getegid",
	"chroot",
	"fcntl",
	"fchdir",
	"fchmod",
	"fchown",
	"mkdir",
	"rmdir",
	"setpgrp",
	"_uname",
	"waitpid",
	"_profil",
	"uadmin",
	"nice",
	"_sigdisp",
	"flock",
	"getpgrp",
	"yield",
	"acct",
	"memalloc",
	"memfree",
	"__netcall",
	"_ftruncate",
	"_nosys68",
	"_nosys69",
	"_nosys70",
	"_nosys71",
	"_select",
	"setgroups",
	"getgroups",
	"getrlimit",
	"setrlimit",
	"setpgid",
	"setsid",
	"getsid",
};

int syscall_args[NR_SYSCALL] = {
	1, //_exit
	VARARGS, //open
	1, //close
	2, //rename
	3, //mknod
	2, //link
	1, //unlink
	3, //read
	3, //write
	3, //_lseek
	1, //chdir
	0, //sync
	2, //access
	2, //chmod
	3, //chown
	2, //_stat
	2, //_fstat
	1, //dup
	0, //getpid
	0, //getppid
	0, //getuid
	1, //umask
	2, //_getfsys
	3, //execve
	3, //_getdirent
	1, //setuid
	1, //setgid
	2, //_time
	2, //stime
	VARARGS, //ioctl
	1, //brk
	1, //sbrk
	2, //_fork
	3, //mount
	2, //_umount
	2, //signal
	2, //dup2
	1, //_pause
	1, //alarm
	2, //kill
	1, //pipe
	0, //getgid
	1, //times
	2, //utime
	0, //geteuid
	0, //getegid
	1, //chroot
	VARARGS, //fcntl
	1, //fchdir
	2, //fchmod
	3, //fchown
	2, //mkdir
	1, //rmdir
	0, //setpgrp
	2, //_uname
	3, //waitpid
	4, //_profil
	3, //uadmin
	1, //nice
	2, //_sigdisp
	2, //flock
	0, //getpgrp
	0, //yield
	1, //acct
	1, //memalloc
	1, //memfree
	1, //netcall
	2, //ftruncate
	0, //nosys 68
	0, //nosys 69
	0, //nosys 70
	0, //nosys 71
	2, //_select
	2, //setgroups
	2, //getgroups
	2, //getrlimit
	2, //setrlimit
	2, //setpgid
	1, //setsid
	1, //getsid
};
