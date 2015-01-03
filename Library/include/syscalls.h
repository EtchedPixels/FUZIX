/* SYSCALLS.H
 */
#ifndef __SYSCALLS_H
#define __SYSCALLS_H
#ifndef __TYPES_H
#include <types.h>
#endif
#ifndef __SIGNAL_H
#include <signal.h>
#endif
#include <sys/stat.h>

extern int errno;
extern int syscall(int callno, ...);

struct  _uzistat
{
	int16_t    st_dev;
	uint16_t   st_ino;
	uint16_t   st_mode;
	uint16_t   st_nlink;
	uint16_t   st_uid;
	uint16_t   st_gid;
	uint16_t   st_rdev;
	uint32_t   st_size;
	uint32_t   st_atime;
	uint32_t   st_mtime;
	uint32_t   st_ctime;
	uint32_t   st_timeh;	/* Time high bytes */
};

struct _uzisysinfoblk {
  uint8_t infosize;		/* For expandability */
  uint8_t banks;		/* Banks in our 64K (and thus pagesize) */
  uint8_t max_open;
  uint8_t nproc;		/* Number of processes */
  uint16_t ticks;		/* Tick rate in HZ */
  uint16_t memk;		/* Memory in KB */
  uint16_t usedk;		/* Used memory in KB */
  uint16_t config;		/* Config flag mask */
  uint16_t loadavg[3];
  uint32_t spare;
};

/*
 *	This is actually overlaid over a blkbuf holding the actual
 *	record in question, and pinned until we umount the fs.
 */
#define __FILESYS_TABSIZE 50

struct _uzifilesys {
    int16_t       s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    uint16_t      s_free[__FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[__FILESYS_TABSIZE];
    uint8_t       s_fmod;
    uint8_t       s_timeh;
    uint32_t      s_time;
    uint16_t      s_tfree;
    uint16_t      s_tinode;
    uint16_t	  s_mntpt;
};

struct hd_geometry {
	uint8_t heads;
	uint8_t sectors;
	uint16_t cylinders;
	uint32_t start;
};
#define HDIO_GETGEO		0x0101
#define HDIO_GET_IDENTITY	0x0102	/* Not yet implemented anywhere */

extern int _exit(int code);
extern int alarm(int16_t secs);
extern int open(const char *path, int flags, ...);
extern int close(int fd);
extern int creat(const char *path, mode_t mode);
extern int mknod(const char *path, mode_t mode, dev_t dev);
extern int link(const char *path, const char *path2);
extern int unlink(const char *path);
extern int read(int fd, char *buf, int len);
extern int write(int fd, const char *buf, int len);
extern int chdir(const char *path);
extern int sync(void);
extern int access(const char *path, int way);
extern int chmod(const char *path, mode_t mode);
extern int chown(const char *path, uid_t owner, gid_t group);
extern int dup(int fd);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern mode_t umask(mode_t);
extern int execve(const char *path, const char *argv[], const char *envp[]);
extern pid_t wait(int *status);
extern int setuid(uid_t uid);
extern int setgid(gid_t gid);
extern int ioctl(int fd, int request,...);
extern int brk(void *addr);
extern void *sbrk(intptr_t increment);
extern pid_t fork(void);
extern int mount(const char *dev, const char *path, int flags);
extern int umount(const char *dev);
extern sighandler_t signal(int signum, sighandler_t sighandler);
extern int dup2(int oldfd, int newfd);
extern int _pause(unsigned int dsecs);
extern int kill(pid_t pid, int sig);
extern int pipe(int *pipefds);
extern gid_t getgid(void);
extern uid_t geteuid(void);
extern gid_t getegid(void);
extern int chroot(const char *path);
extern int fcntl(int fd, int cmd, ...);
extern int fchdir(int fd);
extern int fchmod(int fd, mode_t mode);
extern int fchown(int fd, uid_t owner, gid_t group);
extern int mkdir(const char *path, mode_t mode);
extern int rmdir(const char *path);
extern pid_t setpgrp(void);
extern int waitpid(pid_t pid, int *status, int options);
extern int uadmin(int cmd, int ctrl, void *ptr);
extern int nice(int prio);
extern int rename(const char *path, const char *newpath);
extern int flock(int fd, int op);

/* asm syscall hooks with C wrappers */
extern int _getdirent(int fd, void *buf, int len);
extern int _stat(const char *path, struct _uzistat *s);
extern int _fstat(int fd, struct _uzistat *s);
extern int _getfsys(uint16_t dev, struct _uzifilesys *fs);
extern int _time(time_t *t, uint16_t clock);
extern int _stime(const time_t *t, uint16_t clock);
extern int _times(struct tms *t);
extern int _utime(const char *file, time_t *buf);
extern int _uname(struct _uzisysinfoblk *uzib, int len);
extern int _profil(void *samples, uint16_t offset, uint16_t size, int16_t scale);
extern int _lseek(int fd, off_t *offset, int mode);

/* C library provided syscall emulation */
extern int stat(const char *path, struct stat *s);
extern int fstat(int fd, struct stat *s);
extern int alarm(uint16_t seconds);
extern time_t time(time_t *t);
extern int stime(time_t *t);
extern int times(struct tms *tms);
extern int utime(const char *filename, const struct utimbuf *utim);
extern int uname(struct utsname *buf);
extern int profil(unsigned short *bufbase, size_t bufsize, unsigned long offset,
                  unsigned int scale);


#endif	/* __SYSCALLS_H */


