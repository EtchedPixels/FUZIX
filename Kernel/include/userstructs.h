#ifndef _USERSTRUCTURES_H
#define _USERSTRUCTURES_H

/* Structures shared between kernel and user space.
 *
 * Only use explicitly sized types here --- otherwise differing compiler
 * flags between kernel code and user code can pack them differently.
 */

struct _uzistat
{
	/* Do not change this without also changing stcpy() in syscall_fs.c. */

	int16_t  st_dev;
	uint16_t st_ino;
	uint16_t st_mode;
	uint16_t st_nlink;
	uint16_t st_uid;
	uint16_t st_gid;
	uint16_t st_rdev;
	uint32_t st_size;
	uint32_t st_atime;
	uint32_t st_mtime;
	uint32_t st_ctime;
	uint32_t st_timeh;	/* Time high bytes */
};

#endif

