#undef DEBUG
#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/* N_open is given a string containing a path name in user space,
 * and returns an inode table pointer.  If it returns NULL, the file
 * did not exist.  If the parent existed, and parent is not null,
 * parent will be filled in with the parents inoptr. Otherwise, parent
 * will be set to NULL.
 */

inoptr n_open(char *uname, inoptr *parent)
{
    inoptr r;
    char *tb;

    tb = (char*)tmpbuf(); /* temporary memory to hold kernel's copy of the filename */

    if (ugets(uname, tb, 512) == -1) {
        *parent = NULLINODE;
        return NULLINODE;
    }

#ifdef DEBUG
    kprintf("n_open(\"%s\")\n", tb);
#endif

    r = kn_open(tb, parent);

    brelse(tb);

    return r;
}

inoptr kn_open(char *namep, inoptr *parent)
{
    staticfast inoptr wd;     /* the directory we are currently searching. */
    staticfast inoptr ninode;
    inoptr temp;
    staticfast char *name;

    name = namep;
#ifdef DEBUG
    kprintf("kn_open(\"%s\")\n", name);
#endif

    if(*name == '/')
        wd = udata.u_root;
    else
        wd = udata.u_cwd;

    i_ref(ninode = wd);
    i_ref(ninode);

    for(;;)
    {
        if(ninode)
            magic(ninode);

        /* cheap way to spot rename inside yourself */
        if (udata.u_rename == ninode)
            udata.u_rename = NULLINODE;

        /* See if we are at a mount point */
        if(ninode)
            ninode = srch_mt(ninode);

        while(*name == '/')    /* Skip(possibly repeated) slashes */
            ++name;
        if(!*name)           /* No more components of path? */
            break;
        if(!ninode){
            udata.u_error = ENOENT;
            goto nodir;
        }
        i_deref(wd);
        wd = ninode;
        if(getmode(wd) != F_DIR){
            udata.u_error = ENOTDIR;
            goto nodir;
        }
        if(!(getperm(wd) & OTH_EX)){
            udata.u_error = EPERM;
            goto nodir;
        }

        /* See if we are going up through a mount point */
        if((wd == udata.u_root || (wd->c_num == ROOTINODE && wd->c_dev != root_dev)) &&
                name[0] == '.' && name[1] == '.' &&
                (name[2] == '/' || name[2] == '\0')){
            if (wd == udata.u_root) {
                ninode = wd;
                name += 2;
                continue;
            }
            temp = fs_tab_get(wd->c_dev)->m_fs->s_mntpt;
            ++temp->c_refs;
            i_deref(wd);
            wd = temp;
        }

        ninode = srch_dir(wd, name);

        while(*name != '/' && *name)
            ++name;
    }

    if(parent)
        *parent = wd;
    else
        i_deref(wd);

    if(!(parent || ninode))
        udata.u_error = ENOENT;

    return ninode;

nodir:
    if(parent)
        *parent = NULLINODE;
    i_deref(wd);
    return NULLINODE;
}

/* Srch_dir is given an inode pointer of an open directory and a string
 * containing a filename, and searches the directory for the file.  If
 * it exists, it opens it and returns the inode pointer, otherwise NULL.
 * This depends on the fact that ba_read will return unallocated blocks
 * as zero-filled, and a partially allocated block will be padded with
 * zeroes.
 */

inoptr srch_dir(inoptr wd, char *compname)
{
    int curentry;
    blkno_t curblock;
    struct direct *buf;
    int nblocks;
    uint16_t inum;

    nblocks = (wd->c_node.i_size + BLKMASK) >> BLKSHIFT;

    for(curblock=0; curblock < nblocks; ++curblock) {
        buf = (struct direct *)bread(wd->c_dev, bmap(wd, curblock, 1), 0);
        for(curentry = 0; curentry < (512/DIR_LEN); ++curentry) {
            if(namecomp(compname, buf[curentry].d_name)) {
                inum = buf[curentry & 0x1f].d_ino;
                brelse(buf);
                return i_open(wd->c_dev, inum);
            }
        }
        brelse(buf);
    }
    return NULLINODE;
}


/* Srch_mt sees if the given inode is a mount point. If so it
 * dereferences it, and references and returns a pointer to the
 * root of the mounted filesystem.
 */

inoptr srch_mt(inoptr ino)
{
    uint8_t j;
    struct mount *m = &fs_tab[0];

    for(j=0; j < NMOUNTS; ++j){
        if(m->m_dev != NO_DEVICE &&  m->m_fs->s_mntpt == ino) {
            i_deref(ino);
            return i_open(m->m_dev, ROOTINODE);
        }
        m++;
    };
    return ino;
}


/* I_open is given an inode number and a device number,
 * and makes an entry in the inode table for them, or
 * increases it reference count if it is already there.
 * An inode # of zero means a newly allocated inode.
 */

inoptr i_open(uint16_t dev, uint16_t ino)
{
    struct dinode *buf;
    inoptr nindex, j;
    bool isnew = false;

    if(!validdev(dev))
        panic("i_open: bad dev");

    if(!ino){        /* ino==0 means we want a new one */
        isnew = true;
        ino = i_alloc(dev);
        if(!ino) {
            udata.u_error = ENOSPC;
            return NULLINODE;
        }
    }

    /* Maybe make this DEBUG only eventually - the fs_tab_get cost
       is higher than ideal */
    if(ino < ROOTINODE || ino >= (fs_tab_get(dev)->m_fs->s_isize - 2) * 8) {
        kputs("i_open: bad inode number\n");
        return NULLINODE;
    }

    nindex = NULLINODE;
    for(j=i_tab; j<i_tab+ITABSIZE; j++){
        if(!j->c_refs) // free slot?
            nindex = j;

        if(j->c_dev == dev && j->c_num == ino) {
            nindex = j;
            goto found;
        }
    }
    /* Not already in the table. */

    if(!nindex){      /* No unrefed slots in inode table */
        udata.u_error = ENFILE;
        return(NULLINODE);
    }

    buf =(struct dinode *)bread(dev,(ino>>3)+2, 0);
    memcpy((char *)&(nindex->c_node), (char *)&(buf[ino & 0x07]), 64);
    brelse(buf);

    nindex->c_dev = dev;
    nindex->c_num = ino;
    nindex->c_magic = CMAGIC;

found:
    if(isnew) {
        if(nindex->c_node.i_nlink || nindex->c_node.i_mode & F_MASK)
            goto badino;
    } else {
        if(!(nindex->c_node.i_nlink && nindex->c_node.i_mode & F_MASK))
            goto badino;
    }
    nindex->c_refs++;
    return nindex;

badino:
    kputs("i_open: bad disk inode\n");
    return NULLINODE;
}



/* Ch_link modifies or makes a new entry in the directory for the name
 * and inode pointer given. The directory is searched for oldname.  When
 * found, it is changed to newname, and it inode # is that of *nindex.
 * A oldname of "" matches a unused slot, and a nindex of NULLINODE
 * means an inode # of 0.  A return status of 0 means there was no
 * space left in the filesystem, or a non-empty oldname was not found,
 * or the user did not have write permission.
 */

bool ch_link(inoptr wd, char *oldname, char *newname, inoptr nindex)
{
    struct direct curentry;
    int i;

    if(!(getperm(wd) & OTH_WR))
    {
        udata.u_error = EPERM;
        return false;
    }
    /* Inserting a new blank entry ? */
    if (!*newname && nindex != NULLINODE) {
        udata.u_error = EEXIST;
        return false;
    }

    /* Search the directory for the desired slot. */

    udata.u_offset = 0;

    for(;;)
    {
        udata.u_count = DIR_LEN;
        udata.u_base  =(char *)&curentry;
        udata.u_sysio = true;
        readi(wd, 0);

        /* Read until EOF or name is found.  readi() advances udata.u_offset */
        if(udata.u_count == 0 || namecomp(oldname, curentry.d_name))
            break;
    }

    if(udata.u_count == 0 && *oldname)
        return false;                  /* Entry not found */

    memcpy(curentry.d_name, newname, FILENAME_LEN);
    // pad name with NULLs
    for(i = 0; i < FILENAME_LEN; ++i)
        if(curentry.d_name[i] == '\0')
            break;
    for(; i < FILENAME_LEN; ++i)
        curentry.d_name[i] = '\0';

    if(nindex)
        curentry.d_ino = nindex->c_num;
    else
        curentry.d_ino = 0;

    /* If an existing slot is being used, we must back up the file offset */
    if(udata.u_count){
        udata.u_offset -= DIR_LEN;
    }

    udata.u_count = DIR_LEN;
    udata.u_base  = (char*)&curentry;
    udata.u_sysio = true;
    writei(wd, 0);

    if(udata.u_error)
        return false;

    setftime(wd, A_TIME|M_TIME|C_TIME);     /* Sets c_dirty */

    /* Update file length to next block */
    if(wd->c_node.i_size & BLKMASK)
        wd->c_node.i_size += BLKSIZE - (wd->c_node.i_size & BLKMASK);

    return true; // success
}



/* Filename is given a path name in user space, and copies
 * the final component of it to name(in system space).
 */

void filename(char *userspace_upath, char *name)
{
    char *buf;
    char *ptr;

    buf = tmpbuf();
    if(ugets(userspace_upath, buf, 512)) {
        brelse(buf);
        *name = '\0';
        return;          /* An access violation reading the name */
    }
    ptr = buf;
    while(*ptr)
        ++ptr;
    /* Special case for "...name.../" */
    while(*ptr != '/' && ptr-- > buf);
    ptr++;
    memcpy(name, ptr, FILENAME_LEN);
    brelse(buf);
}


/* Namecomp compares two strings to see if they are the same file name.
 * It stops at FILENAME_LEN chars or a null or a slash. It returns 0 for difference.
 */
bool namecomp(char *n1, char *n2) // return true if n1 == n2
{
    uint8_t n; // do we have enough variables called n?

    n = FILENAME_LEN;
    while(*n1 && *n1 != '/')
    {
        if(*n1++ != *n2++)
            return false; // mismatch
        n--;
        if(n==0)
            return true; // match
    }

    return (*n2 == '\0' || *n2 == '/');
}


/* Newfile is given a pointer to a directory and a name, and creates
 * an entry in the directory for the name, dereferences the parent,
 * and returns a pointer to the new inode.  It allocates an inode
 * number, and creates a new entry in the inode table for the new
 * file, and initializes the inode table entry for the new file.
 * The new file will have one reference, and 0 links to it.
 * Better make sure there isn't already an entry with the same name.
 */

inoptr newfile(inoptr pino, char *name)
{
    inoptr nindex;
    uint8_t j;

    /* First see if parent is writeable */
    if(!(getperm(pino) & OTH_WR))
        goto nogood;

    if(!(nindex = i_open(pino->c_dev, 0)))
        goto nogood;

    /* BUG FIX:  user/group setting was missing  SN */
    nindex->c_node.i_uid = udata.u_euid;
    nindex->c_node.i_gid = udata.u_egid;

    nindex->c_node.i_mode = F_REG;   /* For the time being */
    nindex->c_node.i_nlink = 1;
    nindex->c_node.i_size = 0;
    for(j=0; j <20; j++)
        nindex->c_node.i_addr[j] = 0;
    wr_inode(nindex);

    if(!ch_link(pino, "", name, nindex)) {
        i_deref(nindex);
        goto nogood;
    }
    i_deref(pino);
    return nindex;

nogood:
    i_deref(pino);
    return NULLINODE;
}


/* Check the given device number, and return its address in the mount
 * table.  Also time-stamp the superblock of dev, and mark it modified.
 * Used when freeing and allocating blocks and inodes.
 */

fsptr getdev(uint16_t dev)
{
    struct mount *mnt;
    fsptr devfs;
    time_t t;

    mnt = fs_tab_get(dev);

    if (!mnt || !(devfs = mnt->m_fs) || devfs->s_mounted == 0) {
        panic("getdev: bad dev");
        /* Return needed to persuade SDCC all is ok */
        return NULL;
    }
    rdtime(&t);
    devfs->s_time = t.low;
    devfs->s_timeh = t.high;
    devfs->s_fmod = true;
    ((bufptr)devfs)->bf_dirty = true;
    return devfs;
}


/* Returns true if the magic number of a superblock is corrupt.
*/

bool baddev(fsptr dev)
{
    return(dev->s_mounted != SMOUNTED);
}


/* I_alloc finds an unused inode number, and returns it, or 0
 * if there are no more inodes available.
 */

uint16_t i_alloc(uint16_t devno)
{
    staticfast fsptr dev;
    staticfast blkno_t blk;
    struct dinode *buf;
    staticfast uint16_t j;
    uint16_t k;
    unsigned ino;

    if(baddev(dev = getdev(devno)))
        goto corrupt;

tryagain:
    if(dev->s_ninode) {
        if(!(dev->s_tinode))
            goto corrupt;
        ino = dev->s_inode[--dev->s_ninode];
        if(ino < 2 || ino >=(dev->s_isize-2)*8)
            goto corrupt;
        --dev->s_tinode;
        return(ino);
    }
    /* We must scan the inodes, and fill up the table */

    _sync();           /* Make on-disk inodes consistent */
    k = 0;
    for(blk = 2; blk < dev->s_isize; blk++) {
        buf = (struct dinode *)bread(devno, blk, 0);
        for(j=0; j < 8; j++) {
            if(!(buf[j].i_mode || buf[j].i_nlink))
                dev->s_inode[k++] = 8*(blk-2) + j;
            if(k==FILESYS_TABSIZE) {
                brelse(buf);
                goto done;
            }
        }
        brelse(buf);
    }

done:
    if(!k) {
        if(dev->s_tinode)
            goto corrupt;
        udata.u_error = ENOSPC;
        return(0);
    }
    dev->s_ninode = k;
    goto tryagain;

corrupt:
    kputs("i_alloc: corrupt superblock\n");
    dev->s_mounted = 1;
    udata.u_error = ENOSPC;
    return(0);
}


/* I_free is given a device and inode number, and frees the inode.
 * It is assumed that there are no references to the inode in the
 * inode table or in the filesystem.
 */

void i_free(uint16_t devno, uint16_t ino)
{
    fsptr dev;

    if(baddev(dev = getdev(devno)))
        return;

    if(ino < 2 || ino >=(dev->s_isize-2)*8)
        panic("i_free: bad ino");

    ++dev->s_tinode;
    if(dev->s_ninode < FILESYS_TABSIZE)
        dev->s_inode[dev->s_ninode++] = ino;
}


/* Blk_alloc is given a device number, and allocates an unused block
 * from it. A returned block number of zero means no more blocks.
 */

blkno_t blk_alloc(uint16_t devno)
{
    fsptr dev;
    blkno_t newno;
    blkno_t *buf;
    uint8_t *mbuf;
    int j;

    if(baddev(dev = getdev(devno)))
        goto corrupt2;

    if(dev->s_nfree <= 0 || dev->s_nfree > FILESYS_TABSIZE)
        goto corrupt;

    newno = dev->s_free[--dev->s_nfree];
    if(!newno)
    {
        if(dev->s_tfree != 0)
            goto corrupt;
        udata.u_error = ENOSPC;
        ++dev->s_nfree;
        return(0);
    }

    /* See if we must refill the s_free array */

    if(!dev->s_nfree)
    {
        buf =(blkno_t *)bread(devno, newno, 0);
        dev->s_nfree = buf[0];
        for(j=0; j < FILESYS_TABSIZE; j++)
        {
            dev->s_free[j] = buf[j+1];
        }
        brelse((char *)buf);
    }

    validblk(devno, newno);

    if(!dev->s_tfree)
        goto corrupt;
    --dev->s_tfree;

    /* Zero out the new block */
    mbuf = bread(devno, newno, 2);
    memset(mbuf, 0, 512);
    bawrite(mbuf);
    return newno;

corrupt:
    kputs("blk_alloc: corrupt\n");
    dev->s_mounted = 1;
corrupt2:
    udata.u_error = ENOSPC;
    return 0;
}


/* Blk_free is given a device number and a block number,
 * and frees the block.
 */

void blk_free(uint16_t devno, blkno_t blk)
{
    fsptr dev;
    uint8_t *buf;

    if(!blk)
        return;

    if(baddev(dev = getdev(devno)))
        return;

    validblk(devno, blk);

    if(dev->s_nfree == FILESYS_TABSIZE) {
        buf = bread(devno, blk, 1);
        memcpy(buf, (char *)&(dev->s_nfree), 51*sizeof(int));
        bawrite(buf);
        dev->s_nfree = 0;
    }

    ++dev->s_tfree;
    dev->s_free[(dev->s_nfree)++] = blk;
}


/* Oft_alloc and oft_deref allocate and dereference(and possibly free)
 * entries in the open file table.
 */

int8_t oft_alloc(void)
{
    uint8_t j;

    for(j=0; j < OFTSIZE ; ++j) {
        if(of_tab[j].o_refs == 0) {
            of_tab[j].o_refs = 1;
            of_tab[j].o_inode = NULLINODE;
            return j;
        }
    }
    udata.u_error = ENFILE;
    return -1;
}


void oft_deref(int8_t of)
{
    struct oft *ofptr;

    ofptr = of_tab + of;
    if(!(--ofptr->o_refs) && ofptr->o_inode) {
        i_deref(ofptr->o_inode);
        ofptr->o_inode = NULLINODE;
    }
}


/* Uf_alloc finds an unused slot in the user file table.
*/

int8_t uf_alloc_n(int base)
{
    uint8_t j;

    for(j=base; j < UFTSIZE ; ++j) {
        if(udata.u_files[j] == NO_FILE) {
            return j;
        }
    }
    udata.u_error = ENFILE;
    return -1;
}


int8_t uf_alloc(void)
{
    return uf_alloc_n(0);
}



/* I_ref increases the reference count of the given inode table entry.
*/

void i_ref(inoptr ino)
{
    ino->c_refs++;
}


/* I_deref decreases the reference count of an inode, and frees it from
 * the table if there are no more references to it.  If it also has no
 * links, the inode itself and its blocks(if not a device) is freed.
 */

void i_deref(inoptr ino)
{
    magic(ino);

    if(!ino->c_refs)
        panic("inode freed.");

    if((ino->c_node.i_mode & F_MASK) == F_PIPE)
        wakeup((char *)ino);

    /* If the inode has no links and no refs, it must have
       its blocks freed. */

    if(!(--ino->c_refs || ino->c_node.i_nlink))
        /*
           SN (mcy)
           */
        if(((ino->c_node.i_mode & F_MASK) == F_REG) ||
                ((ino->c_node.i_mode & F_MASK) == F_DIR) ||
                ((ino->c_node.i_mode & F_MASK) == F_PIPE))
            f_trunc(ino);

    /* If the inode was modified, we must write it to disk. */
    if(!(ino->c_refs) && ino->c_dirty)
    {
        if(!(ino->c_node.i_nlink))
        {
            ino->c_node.i_mode = 0;
            i_free(ino->c_dev, ino->c_num);
        }
        wr_inode(ino);
    }
}


/* Wr_inode writes out the given inode in the inode table out to disk,
 * and resets its dirty bit.
 */

void wr_inode(inoptr ino)
{
    struct dinode *buf;
    blkno_t blkno;

    magic(ino);

    blkno =(ino->c_num >> 3) + 2;
    buf =(struct dinode *)bread(ino->c_dev, blkno,0);
    memcpy((char *)((char **)&buf[ino->c_num & 0x07]), (char *)(&ino->c_node), 64);
    bfree((bufptr)buf, 2);
    ino->c_dirty = false;
}


/* isdevice(ino) returns true if ino points to a device */
bool isdevice(inoptr ino)
{
    return (ino->c_node.i_mode & F_CDEV);
}


/* This returns the device number of an inode representing a device */
uint16_t devnum(inoptr ino)
{
    return (uint16_t)ino->c_node.i_addr[0];
}


/* F_trunc frees all the blocks associated with the file, if it
 * is a disk file.
 */
void f_trunc(inoptr ino)
{
    uint16_t dev;
    int8_t j;

    dev = ino->c_dev;

    /* First deallocate the double indirect blocks */
    freeblk(dev, ino->c_node.i_addr[19], 2);

    /* Also deallocate the indirect blocks */
    freeblk(dev, ino->c_node.i_addr[18], 1);

    /* Finally, free the direct blocks */
    for(j=17; j >= 0; --j)
        freeblk(dev, ino->c_node.i_addr[j], 0);

    memset((char *)ino->c_node.i_addr, 0, sizeof(ino->c_node.i_addr));

    ino->c_dirty = true;
    ino->c_node.i_size = 0;
}


/* Companion function to f_trunc(). */
void freeblk(uint16_t dev, blkno_t blk, uint8_t level)
{
    blkno_t *buf;
    int8_t j;

    if(!blk)
        return;

    if(level){
        buf = (blkno_t *)bread(dev, blk, 0);
        for(j=255; j >= 0; --j)
            freeblk(dev, buf[j], level-1);
        brelse((char *)buf);
    }
    blk_free(dev, blk);
}



/* Changes: blk_alloc zeroes block it allocates */
/*
 * Bmap defines the structure of file system storage by returning
 * the physical block number on a device given the inode and the
 * logical block number in a file.  The block is zeroed if created.
 */
blkno_t bmap(inoptr ip, blkno_t bn, int rwflg)
{
    int i;
    bufptr bp;
    int j;
    blkno_t nb;
    int sh;
    uint16_t dev;

    if(getmode(ip) == F_BDEV)
        return(bn);

    dev = ip->c_dev;

    /* blocks 0..17 are direct blocks
    */
    if(bn < 18) {
        nb = ip->c_node.i_addr[bn];
        if(nb == 0) {
            if(rwflg ||(nb = blk_alloc(dev))==0)
                return(NULLBLK);
            ip->c_node.i_addr[bn] = nb;
            ip->c_dirty = true;
        }
        return(nb);
    }

    /* addresses 18 and 19 have single and double indirect blocks.
     * the first step is to determine how many levels of indirection.
     */
    bn -= 18;
    sh = 0;
    j = 2;
    if(bn & 0xff00){       /* bn > 255  so double indirect */
        sh = 8;
        bn -= 256;
        j = 1;
    }

    /* fetch the address from the inode
     * Create the first indirect block if needed.
     */
    if(!(nb = ip->c_node.i_addr[20-j]))
    {
        if(rwflg || !(nb = blk_alloc(dev)))
            return(NULLBLK);
        ip->c_node.i_addr[20-j] = nb;
        ip->c_dirty = true;
    }

    /* fetch through the indirect blocks
    */
    for(; j<=2; j++) {
        bp =(bufptr)bread(dev, nb, 0);
        /******
          if(bp->bf_error) {
          brelse(bp);
          return((blkno_t)0);
          }
         ******/
        i =(bn>>sh) & 0xff;
        if((nb =((blkno_t *)bp)[i]) != 0)
            brelse(bp);
        else
        {
            if(rwflg || !(nb = blk_alloc(dev))) {
                brelse(bp);
                return(NULLBLK);
            }
            ((blkno_t *)bp)[i] = nb;
            bawrite(bp);
        }
        sh -= 8;
    }
    return(nb);
}



/* Validblk panics if the given block number is not a valid
 *  data block for the given device.
 */
void validblk(uint16_t dev, blkno_t num)
{
    struct mount *mnt;
    fsptr devptr;

    mnt = fs_tab_get(dev);

    if(mnt == NULL || !(devptr = mnt->m_fs) || devptr->s_mounted == 0) {
        panic("validblk: not mounted");
        return;
    }

    if(num < devptr->s_isize || num >= devptr->s_fsize)
        panic("validblk: invalid blk");
}


/* This returns the inode pointer associated with a user's file
 * descriptor, checking for valid data structures.
 */
inoptr getinode(uint8_t uindex)
{
    uint8_t oftindex;
    inoptr inoindex;

    if(uindex >= UFTSIZE || udata.u_files[uindex] == NO_FILE) {
        udata.u_error = EBADF;
        return NULLINODE;
    }

    oftindex = udata.u_files[uindex];

    if(oftindex >= OFTSIZE || oftindex == NO_FILE)
        panic("getinode: bad desc table");

    if((inoindex = of_tab[oftindex].o_inode) < i_tab || inoindex >= i_tab+ITABSIZE)
        panic("getinode: bad OFT");

    magic(inoindex);
    return(inoindex);
}


/* Super returns true if we are the superuser */
bool super(void)
{
    return(udata.u_euid == 0);
}

/* Similar but this helper sets the error code */
bool esuper(void)
{
    if (udata.u_euid) {
        udata.u_error = EPERM;
        return -1;
    }
    return 0;
}

/* Getperm looks at the given inode and the effective user/group ids,
 * and returns the effective permissions in the low-order 3 bits.
 */
uint8_t getperm(inoptr ino)
{
    int mode;

    if(super())
        return(07);

    mode = ino->c_node.i_mode;
    if(ino->c_node.i_uid == udata.u_euid)
        mode >>= 6;
    else if(ino->c_node.i_gid == udata.u_egid)
        mode >>= 3;

    return(mode & 07);
}


/* This sets the times of the given inode, according to the flags.
*/
void setftime(inoptr ino, uint8_t flag)
{
    ino->c_dirty = true;

    if(flag & A_TIME)
        rdtime32(&(ino->c_node.i_atime));
    if(flag & M_TIME)
        rdtime32(&(ino->c_node.i_mtime));
    if(flag & C_TIME)
        rdtime32(&(ino->c_node.i_ctime));
}


uint16_t getmode(inoptr ino)
{
    return(ino->c_node.i_mode & F_MASK);
}


static struct mount *newfstab(void)
{
    struct mount *m = fs_tab;
    int i;
    for (i = 0; i < NMOUNTS; i++) {
        if (m->m_dev == NO_DEVICE)
            return m;
        m++;
    }
    return NULL;
}

struct mount *fs_tab_get(uint16_t dev)
{
    struct mount *m = fs_tab;
    int i;
    for (i = 0; i < NMOUNTS; i++) {
        if (m->m_dev == dev)
            return m;
        m++;
    }
    return NULL;
}

/* Fmount places the given device in the mount table with mount point ino.
*/
bool fmount(uint16_t dev, inoptr ino, uint16_t flags)
{
    struct mount *m;
    struct filesys *fp;

    if(d_open(dev, 0) != 0)
        panic("fmount: can't open filesystem");

    m = newfstab();
    if (m == NULL) {
        udata.u_error = EMFILE;
        return true;	/* Table is full */
    }
    /* Pin the buffer at dev 0 blk 1, it will only be released
       by umount */
    fp = (filesys *)bread(dev, 1, 0);

    //   kprintf("fp->s_mounted=0x%x, fp->s_isize=0x%x, fp->s_fsize=0x%x\n", 
    //   fp->s_mounted, fp->s_isize, fp->s_fsize);

    /* See if there really is a filesystem on the device */
    if(fp->s_mounted != SMOUNTED  ||  fp->s_isize >= fp->s_fsize) {
        udata.u_error = EINVAL;
        bfree((bufptr)fp, 0);
        return true; // failure
    }

    fp->s_mntpt = ino;
    if(ino)
        ++ino->c_refs;
    m->m_flags = flags;
    /* Makes our entry findable */
    m->m_fs = fp;
    m->m_dev = dev;
    return false; // success
}


void magic(inoptr ino)
{
    if(ino->c_magic != CMAGIC)
        panic("corrupt inode");
}
