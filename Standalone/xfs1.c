/**************************************************
UZI (Unix Z80 Implementation) Utilities:  xfs1.c
***************************************************/

/*LINTLIBRARY*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "fuzix_fs.h"

void fs_init(void)
{
    udata.u_euid = 0;
    udata.u_insys = 1;
}

void xfs_init(int bootdev)
{
    register char *j;
    inoptr i_open();

    fs_init();
    bufinit();

    /* User's file table */
    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
        *j = -1;

    /* Mount the root device */
    if (fmount(ROOTDEV, NULLINODE))
        panic("no filesys");

    ifnot (root = i_open(ROOTDEV,ROOTINODE))
        panic("no root");

    i_ref(udata.u_cwd = root);
}


void xfs_end(void)
{
    register int16_t j;

    for (j=0; j < UFTSIZE; ++j)
    {
        ifnot (udata.u_files[j] & 0x80)  /* Portable equivalent of == -1 */
            doclose(j);
    }
}


int _open(char *name, int16_t flag)
{
    int16_t uindex;
    register int16_t oftindex;
    register inoptr ino;
    register int16_t perm;
    inoptr n_open();
    int    getperm(), getmode(), isdevice(), d_open();
    int    uf_alloc(), oft_alloc();

    udata.u_error = 0;

    if (flag < 0 || flag > 2)
    {
        udata.u_error = EINVAL;
        return (-1);
    }
    if ((uindex = uf_alloc()) == -1)
        return (-1);

    if ((oftindex = oft_alloc()) == -1)
        goto nooft;

    ifnot(ino = n_open(name,NULLINOPTR))
        goto cantopen;

    of_tab[oftindex].o_inode = ino;

    perm = getperm(ino);
    if (((flag == FO_RDONLY || flag == FO_RDWR) && !(perm & OTH_RD)) ||
            ((flag == FO_WRONLY || flag == FO_RDWR) && !(perm & OTH_WR)))
    {
        udata.u_error = EPERM;
        goto cantopen;
    }

    if (getmode(ino) == F_DIR &&
            (flag == FO_WRONLY || flag == FO_RDWR))
    {
        udata.u_error = EISDIR;
        goto cantopen;
    }

    if (isdevice(ino)) // && d_open((int)ino->c_node.i_addr[0]) != 0)
    {
        udata.u_error = ENXIO;
        goto cantopen;
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr = 0;
    of_tab[oftindex].o_access = flag;

    return (uindex);

cantopen:
    oft_deref(oftindex);  /* This will call i_deref() */
nooft:
    udata.u_files[uindex] = -1;
    return (-1);
}





int doclose(int16_t uindex)
{
    register int16_t oftindex;
    inoptr ino;
    inoptr getinode();
    int    isdevice();

    udata.u_error = 0;
    ifnot(ino = getinode(uindex))
        return(-1);
    oftindex = udata.u_files[uindex];

    //if (isdevice(ino)
    //        /* && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1 */ )
    //    d_close((int)(ino->c_node.i_addr[0]));

    udata.u_files[uindex] = -1;
    oft_deref(oftindex);

    return(0);
}

int _close(int16_t uindex)
{
    udata.u_error = 0;
    return(doclose(uindex));
}



int _creat(char *name, int16_t mode)
{
    register inoptr ino;
    register int16_t uindex;
    register int16_t oftindex;
    inoptr parent;
    register int16_t j;
    inoptr n_open();
    inoptr newfile();
    int    getperm(), getmode(), uf_alloc(), oft_alloc();

    udata.u_error = 0;
    parent = NULLINODE;

    if ((uindex = uf_alloc()) == -1)
        return (-1);
    if ((oftindex = oft_alloc()) == -1)
        return (-1);

    ino = n_open(name,&parent);
    if (ino)
    {
        i_deref(parent);
        if (getmode(ino) == F_DIR)
        {
            i_deref(ino);
            udata.u_error = EISDIR;
            goto nogood;
        }
        ifnot (getperm(ino) & OTH_WR)
        {
            i_deref(ino);
            udata.u_error = EACCES;
            goto nogood;
        }
        if (getmode(ino) == F_REG)
        {
            /* Truncate the file to zero length */
            f_trunc(ino);
            /* Reset any oft pointers */
            for (j=0; j < OFTSIZE; ++j)
                if (of_tab[j].o_inode == ino)
                    of_tab[j].o_ptr = 0;
        }
    }
    else
    {
        if (parent && (ino = newfile(parent,name)))
            /* Parent was derefed in newfile */
        {
            ino->c_node.i_mode = swizzle16(F_REG | (mode & MODE_MASK & ~udata.u_mask));
            setftime(ino, A_TIME|M_TIME|C_TIME);
            /* The rest of the inode is initialized in newfile() */
            wr_inode(ino);
        }
        else
        {
            /* Doesn't exist and can't make it */
            if (parent)
                i_deref(parent);
            goto nogood;
        }
    }
    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr = 0;
    of_tab[oftindex].o_inode = ino;
    of_tab[oftindex].o_access = FO_WRONLY;

    return (uindex);

nogood:
    oft_deref(oftindex);
    return (-1);
}



int _link( char *name1, char *name2)
{
    register inoptr ino;
    register inoptr ino2;
    inoptr parent2;
    char *filename();
    inoptr n_open();
    int    ch_link(), getmode(), super();

    udata.u_error = 0;
    ifnot (ino = n_open(name1,NULLINOPTR))
        return(-1);

    if (getmode(ino) == F_DIR && !super())
    {
        udata.u_error = EPERM;
        goto nogood;
    }

    /* Make sure file2 doesn't exist, and get its parent */
    if ((ino2 = n_open(name2,&parent2)))
    {
        i_deref(ino2);
        i_deref(parent2);
        udata.u_error = EEXIST;
        goto nogood;
    }

    ifnot (parent2)
        goto nogood;

    if (ino->c_dev != parent2->c_dev)
    {
        i_deref(parent2);
        udata.u_error = EXDEV;
        goto nogood;
    }

    if (ch_link(parent2,"",filename(name2),ino) == 0)
        goto nogood;

    /* Update the link count. */
    ino->c_node.i_nlink = swizzle16(swizzle16(ino->c_node.i_nlink)+1);
    wr_inode(ino);
    setftime(ino, C_TIME);

    i_deref(parent2);
    i_deref(ino);
    return(0);

nogood:
    i_deref(ino);
    return(-1);
}



int _unlink(char *path)
{
    register inoptr ino;
    inoptr pino;
    char *filename();
    /*--    inoptr i_open();--*/
    inoptr n_open();
    int    getmode(), ch_link(), super();

    udata.u_error = 0;
    ino = n_open(path,&pino);

    ifnot (pino && ino)
    {
        udata.u_error = ENOENT;
        return (-1);
    }

    if (getmode(ino) == F_DIR && !super())
    {
        udata.u_error = EPERM;
        goto nogood;
    }

    /* Remove the directory entry */

    if (ch_link(pino,filename(path),"",NULLINODE) == 0)
        goto nogood;

    /* Decrease the link count of the inode */

    if (ino->c_node.i_nlink == 0)
    {
        ino->c_node.i_nlink = swizzle16(swizzle16(ino->c_node.i_nlink)+2);
        printf("_unlink: bad nlink\n");
    } else
        ino->c_node.i_nlink = swizzle16(swizzle16(ino->c_node.i_nlink)-1);
    setftime(ino, C_TIME);
    i_deref(pino);
    i_deref(ino);
    return(0);
nogood:
    i_deref(pino);
    i_deref(ino);
    return(-1);
}



int _read( int16_t d, char *buf, uint16_t nbytes)
{
    register inoptr ino;
    inoptr rwsetup();

    udata.u_error = 0;
    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(1, d, buf, nbytes)) == NULLINODE)
        return (-1);   /* bomb out if error */

    readi(ino);
    updoff(d);

    return (udata.u_count);
}



int _write( int16_t d, char *buf, uint16_t nbytes)
{
    register inoptr ino;
    /*--    off_t *offp;--*/
    inoptr rwsetup();

    udata.u_error = 0;
    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(0, d, buf, nbytes)) == NULLINODE)
        return (-1);   /* bomb out if error */

    writei(ino);
    updoff(d);

    return (udata.u_count);
}



inoptr rwsetup( int rwflag, int d, char *buf, int nbytes)
{
    register inoptr ino;
    register struct oft *oftp;
    inoptr getinode();

    udata.u_base = buf;
    udata.u_count = nbytes;

    if ((ino = getinode(d)) == NULLINODE)
        return (NULLINODE);

    oftp = of_tab + udata.u_files[d];
    if (oftp->o_access == (rwflag ? FO_WRONLY : FO_RDONLY))
    {
        udata.u_error = EBADF;
        return (NULLINODE);
    }

    setftime(ino, rwflag ? A_TIME : (A_TIME | M_TIME | C_TIME));

    /* Initialize u_offset from file pointer */
    udata.u_offset = oftp->o_ptr;

    return (ino);
}



int psize(inoptr ino)
{
    return swizzle32(ino->c_node.i_size);
}
