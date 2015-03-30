/**************************************************
  UZI (Unix Z80 Implementation) Utilities:  xfs1a1.c
 ***************************************************/

 /*LINTLIBRARY*/
#include <stddef.h>
#include <stdint.h>
#include <strings.h>
#include "fuzix_fs.h"


struct cinode i_tab[ITABSIZE];
struct filesys fs_tab[1];

int valadr(char *base, uint16_t size)
{
	return (1);
}


int _mknod(char *name, int16_t mode, int16_t dev)
{
	register inoptr ino;
	inoptr parent;

	udata.u_error = 0;
	ifnot(super()) {
		udata.u_error = EPERM;
		return (-1);
	}

	if ((ino = n_open(name, &parent))) {
		udata.u_error = EEXIST;
		goto nogood;
	}

	ifnot(parent) {
		udata.u_error = ENOENT;
		return (-1);
	}

	ifnot(ino = newfile(parent, name))
	    goto nogood2;

	/* Initialize mode and dev */
	ino->c_node.i_mode = swizzle16(mode & ~udata.u_mask);
	ino->c_node.i_addr[0] = swizzle16(isdevice(ino) ? dev : 0);
	setftime(ino, A_TIME | M_TIME | C_TIME);
	wr_inode(ino);

	i_deref(ino);
	return (0);

      nogood:
	i_deref(ino);
      nogood2:
	i_deref(parent);
	return (-1);
}

void _sync(void)
{
	int j;
	inoptr ino;
	char *buf;

	/* Write out modified inodes */

	udata.u_error = 0;
	for (ino = i_tab; ino < i_tab + ITABSIZE; ++ino)
		if ((ino->c_refs) > 0 && ino->c_dirty != 0) {
			wr_inode(ino);
			ino->c_dirty = 0;
		}

	/* Write out modified super blocks */
	/* This fills the rest of the super block with garbage. */

	for (j = 0; j < NDEVS; ++j) {
		if (swizzle16(fs_tab[j].s_mounted) == SMOUNTED
		    && fs_tab[j].s_fmod) {
			fs_tab[j].s_fmod = 0;
			buf = bread(j, 1, 1);
			bcopy((char *) &fs_tab[j], buf, 512);
			bfree((bufptr) buf, 2);
		}
	}
	bufsync();		/* Clear buffer pool */
}

int _chdir(char *dir)
{
	register inoptr newcwd;
	inoptr n_open();
	int getmode();

	udata.u_error = 0;
	ifnot(newcwd = n_open(dir, NULLINOPTR))
	    return (-1);

	if (getmode(newcwd) != F_DIR) {
		udata.u_error = ENOTDIR;
		i_deref(newcwd);
		return (-1);
	}
	i_deref(udata.u_cwd);
	udata.u_cwd = newcwd;
	return (0);
}

int min(int a, int b)
{
	return (a < b ? a : b);
}

int _access(char *path, int16_t mode)
{
	register inoptr ino;
	register int16_t euid;
	register int16_t egid;
	register int16_t retval;
	inoptr n_open();
	int getperm();

	udata.u_error = 0;
	if ((mode & 07) && !*(path)) {
		udata.u_error = ENOENT;
		return (-1);
	}

	/* Temporarily make eff. id real id. */
	euid = udata.u_euid;
	egid = udata.u_egid;
	udata.u_euid = 0;	// udata.u_ptab->p_uid;
	udata.u_egid = 0;	// udata.u_gid;

	ifnot(ino = n_open(path, NULLINOPTR)) {
		retval = -1;
		goto nogood;
	}

	retval = 0;
	if (~getperm(ino) & (mode & 07)) {
		udata.u_error = EPERM;
		retval = -1;
	}

	i_deref(ino);
nogood:
	udata.u_euid = euid;
	udata.u_egid = egid;

	return (retval);
}

int _chmod(char *path, int16_t mode)
{
	inoptr ino;
	inoptr n_open();
	int super();

	udata.u_error = 0;
	ifnot(ino = n_open(path, NULLINOPTR))
	    return (-1);

	if (swizzle16(ino->c_node.i_uid) != udata.u_euid && !super()) {
		i_deref(ino);
		udata.u_error = EPERM;
		return (-1);
	}
	ino->c_node.i_mode =
	    swizzle16((mode & MODE_MASK) |
		      (swizzle16(ino->c_node.i_mode) & F_MASK));
	setftime(ino, C_TIME);
	i_deref(ino);
	return (0);
}

int _chown(char *path, int owner, int group)
{
	register inoptr ino;
	inoptr n_open();
	int super();

	udata.u_error = 0;
	ifnot(ino = n_open(path, NULLINOPTR))
	    return (-1);

	if (swizzle16(ino->c_node.i_uid) != udata.u_euid && !super()) {
		i_deref(ino);
		udata.u_error = EPERM;
		return (-1);
	}

	ino->c_node.i_uid = swizzle16(owner);
	ino->c_node.i_gid = swizzle16(group);
	setftime(ino, C_TIME);
	i_deref(ino);
	return (0);
}

int _stat(char *path, struct uzi_stat *buf)
{
	register inoptr ino;

	udata.u_error = 0;
	ifnot(valadr((char *) buf, sizeof(struct uzi_stat))
	      && (ino = n_open(path, NULLINOPTR))) {
		return (-1);
	}
	stcpy(ino, buf);
	i_deref(ino);
	return (0);
}

int _fstat(int16_t fd, struct uzi_stat *buf)
{
	register inoptr ino;

	udata.u_error = 0;
	ifnot(valadr((char *) buf, sizeof(struct uzi_stat)))
	    return (-1);

	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);

	stcpy(ino, buf);
	return (0);
}

/* Utility for stat and fstat */
void stcpy(inoptr ino, struct uzi_stat *buf)
{
	struct uzi_stat *b = (struct uzi_stat *) buf;

	b->st_dev = swizzle16(ino->c_dev);
	b->st_ino = swizzle16(ino->c_num);
	b->st_mode = swizzle16(ino->c_node.i_mode);
	b->st_nlink = swizzle16(ino->c_node.i_nlink);
	b->st_uid = swizzle16(ino->c_node.i_uid);
	b->st_gid = swizzle16(ino->c_node.i_gid);

	b->st_rdev = swizzle16(ino->c_node.i_addr[0]);

	b->st_size = swizzle32(ino->c_node.i_size);
	b->fst_atime = swizzle32(ino->c_node.i_atime);
	b->fst_mtime = swizzle32(ino->c_node.i_mtime);
	b->fst_ctime = swizzle32(ino->c_node.i_ctime);
}

int _dup(int16_t oldd)
{
	register int newd;
	inoptr getinode();
	int uf_alloc();

	udata.u_error = 0;
	if (getinode(oldd) == NULLINODE)
		return (-1);

	if ((newd = uf_alloc()) == -1)
		return (-1);

	udata.u_files[newd] = udata.u_files[oldd];
	++of_tab[udata.u_files[oldd]].o_refs;

	return (newd);
}

int _dup2(int16_t oldd, int16_t newd)
{
	inoptr getinode();

	udata.u_error = 0;
	if (getinode(oldd) == NULLINODE)
		return (-1);

	if (newd < 0 || newd >= UFTSIZE) {
		udata.u_error = EBADF;
		return (-1);
	}

	ifnot(udata.u_files[newd] & 0x80)
	    doclose(newd);

	udata.u_files[newd] = udata.u_files[oldd];
	++of_tab[udata.u_files[oldd]].o_refs;

	return (0);
}

int _umask(int mask)
{
	register int omask;

	udata.u_error = 0;
	omask = udata.u_mask;
	udata.u_mask = mask & 0777;
	return (omask);
}

/* Special system call returns super-block of given filesystem for
 * users to determine free space, etc.  Should be replaced with a
 * sync() followed by a read of block 1 of the device.
 */

int _getfsys(int dev, char *buf)
{
	udata.u_error = 0;
	if (dev < 0 || dev >= NDEVS
	    || swizzle16(fs_tab[dev].s_mounted) != SMOUNTED) {
		udata.u_error = ENXIO;
		return (-1);
	}

	/* FIXME: endiam swapping here */
	bcopy((char *) &fs_tab[dev], (char *) buf, sizeof(struct filesys));
	return (0);
}

int _ioctl(int fd, int request, char *data)
{
	register inoptr ino;
	// register int dev;

	udata.u_error = 0;
	if ((ino = getinode(fd)) == NULLINODE)
		return (-1);

	ifnot(isdevice(ino)) {
		udata.u_error = ENOTTY;
		return (-1);
	}

	ifnot(getperm(ino) & OTH_WR) {
		udata.u_error = EPERM;
		return (-1);
	}

	// dev = ino->c_node.i_addr[0];

	// if (d_ioctl(dev, request,data))
	//     return(-1);

	return (0);
}

int _mount(char *spec, char *dir, int rwflag)
{
	register inoptr sino, dino;
	register int dev;

	udata.u_error = 0;
	ifnot(super()) {
		udata.u_error = EPERM;
		return (-1);
	}
	ifnot(sino = n_open(spec, NULLINOPTR))
	    return (-1);

	ifnot(dino = n_open(dir, NULLINOPTR)) {
		i_deref(sino);
		return (-1);
	}
	if (getmode(sino) != F_BDEV) {
		udata.u_error = ENOTBLK;
		goto nogood;
	}
	if (getmode(dino) != F_DIR) {
		udata.u_error = ENOTDIR;
		goto nogood;
	}
	dev = (int) swizzle16(sino->c_node.i_addr[0]);

	if (dev >= NDEVS)	// || d_open(dev))
	{
		udata.u_error = ENXIO;
		goto nogood;
	}
	if (fs_tab[dev].s_mounted || dino->c_refs != 1
	    || dino->c_num == ROOTINODE) {
		udata.u_error = EBUSY;
		goto nogood;
	}
	_sync();

	if (fmount(dev, dino)) {
		udata.u_error = EBUSY;
		goto nogood;
	}
	i_deref(dino);
	i_deref(sino);
	return (0);
      nogood:
	i_deref(dino);
	i_deref(sino);
	return (-1);
}

int _umount(char *spec)
{
	register inoptr sino;
	register int dev;
	register inoptr ptr;

	udata.u_error = 0;
	ifnot(super()) {
		udata.u_error = EPERM;
		return (-1);
	}

	ifnot(sino = n_open(spec, NULLINOPTR))
	    return (-1);

	if (getmode(sino) != F_BDEV) {
		udata.u_error = ENOTBLK;
		goto nogood;
	}

	dev = (int) swizzle16(sino->c_node.i_addr[0]);
	//ifnot (validdev(dev))
	//{
	//    udata.u_error = ENXIO;
	//    goto nogood;
	//}

	if (!fs_tab[dev].s_mounted) {
		udata.u_error = EINVAL;
		goto nogood;
	}

	for (ptr = i_tab; ptr < i_tab + ITABSIZE; ++ptr)
		if (ptr->c_refs > 0 && ptr->c_dev == dev) {
			udata.u_error = EBUSY;
			goto nogood;
		}
	_sync();
	fs_tab[dev].s_mounted = 0;
	i_deref(fs_tab[dev].s_mntpt);

	i_deref(sino);
	return (0);

      nogood:
	i_deref(sino);
	return (-1);
}
