#undef DEBUG
#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/*
 * There are only two places in the core kernel that know about buffer
 * data and manipulate it directly. This is one of them, and  mm.c is the
 * other. Please keep it that way if at all possible because at some point
 * we will begin supporting out of memory map buffers.
 */

/* N_open is given a string containing a path name in user space,
 * and returns an inode table pointer.  If it returns NULL, the file
 * did not exist.  If the parent existed, and parent is not null,
 * parent will be filled in with the parents inoptr. Otherwise, parent
 * will be set to NULL.
 * The last node parsed is saved in lastname and is useful to some system
 * calls as they want a parent and to create the new node.
 */

uint8_t lastname[31];

static uint_fast8_t n_open_fault;
static uint_fast8_t n_fault_type;
static uint8_t *name, *nameend;

static uint8_t getcf(void)
{
    if (name == nameend) {
        udata.u_error = n_fault_type;
        n_open_fault = 1;
        return 0;
    }
    return (uint8_t)_ugetc(name);
}

inoptr n_open(uint8_t *namep, inoptr *parent)
{
    staticfast inoptr wd;     /* the directory we are currently searching. */
    staticfast inoptr ninode;
    inoptr temp;
    uint8_t c;
    uint8_t *fp;
    usize_t len;

    if (parent)
        *parent = NULLINODE;

    /* Check the user address and length. If it's shorter than 512 bytes this
       is fine, but set nameeend accordingly. This allows us to use _ugetc
       in the hot path which saves us a ton of cycles */
    len = valaddr_r(namep, 512);
    if (len == 0)
        return NULLINODE;

    name = namep;
    nameend = namep + len;
    n_open_fault = 0;

    /* What error do we return if we hit nameend - are we overlong, or out
       of memory space */
    if (len == 512)
        n_fault_type = ENAMETOOLONG;
    else
        n_fault_type = EACCES;

    if(getcf() == '/')
        wd = udata.u_root;
    else
        wd = udata.u_cwd;

    ninode = i_ref(wd);
    i_ref(ninode);

    for(;;)
    {
        /* ninode is the inode we are walking from at this point and wd
           the parent. They may be the same. We hold one reference to each */
        if(ninode)
            magic(ninode);

        /* cheap way to spot rename inside yourself */
        if (udata.u_rename == ninode)
            udata.u_rename = NULLINODE;

        /* See if we are at a mount point.
           If we are a mount point then we swap the parent inode for
           the child inode and swap the reference to the child
           Q: could we set an incore inode flag for mountpoint to speed
              this up ? */
        if(ninode)
            ninode = srch_mt(ninode);

        /* Skip any slashes between nodes. The standards say there can be
           multiple slashes */
        while((c = getcf()) == '/')
            ++name;
        /* It is acceptable to end a file path with / */
        if(!c || n_open_fault)           /* No more components of path? */
            break;

        /* If we failed to find our node we are done */
        if(!ninode){
            udata.u_error = ENOENT;
            goto nodir;
        }
        /* Drop the reference to the old parent */
        i_deref(wd);
        /* Make the parent our own node. We now hold a single reference to
           wd/ninode */
        wd = ninode;
        /* If we are still searching and the parent node is not a directory
           we are done and we failed */
        if(getmode(wd) != MODE_R(F_DIR)){
            udata.u_error = ENOTDIR;
            goto nodir;
        }
        /* If we are now allowed to access the directory then we are done */
        if(!(getperm(wd) & OTH_EX)){
            udata.u_error = EACCES;
            goto nodir;
        }

        /* Walk the filename until / or end. Store the filename of up to
           30 characters in lastname which is also used by our callers. It
           is permissible to give longer name that matches the first 30 */
        fp = lastname;
        while((c = getcf()) != '\0') {
            if (c == '/')
                break;
            if (fp != lastname + 30)
                *fp++ = c;
            ++name;
        }
        /* Terminate the lastname buffer with \0 */
        *fp = 0;
        /* We are going up through a mount point if
           either:
           - We are accessing the root inode
           - We are accessing the root inode number of a device
           and:
           - Our path is ..

           FIXME: re-order tests for speed */
        if((wd == udata.u_root || (wd->c_num == ROOTINODE && wd->c_dev != root_dev)) &&
                lastname[0] == '.' && lastname[1] == '.' && lastname[2] == '\0') {
            /* We are doing /../ */
            if (wd == udata.u_root) {
                i_ref(wd);
                continue;
            }
            /* Find the mount point inode, which is hidden by the mount */
            temp = fs_tab[wd->c_super].m_mntpt;
            /* Take a reference to it */
            i_ref(temp);
            /* Drop the old directory */
            i_deref(wd);
            /* Fall through so we walk the mount point directory .. entry */
            wd = temp;
        }
        /* Find the entry in the directory. ninode will be NULL if we failed or
           valid and referenced if it existed */
        ninode = srch_dir(wd, lastname);
    }
    /* If we faulted then treat it as invalid */
    if (n_open_fault) {
        udata.u_error = n_fault_type;
        goto nodir;
    }

    /* Return the parent node if requested. This is needed by callers that
       do directory manipulation */
    if(parent)
        *parent = wd;
    else
        i_deref(wd);
    /* Check if we failed */
    if(!(parent || ninode))
        udata.u_error = ENOENT;
    /* Return the target node if found. NULL with a valid parent is quite
       possible and indicates the directory exists but the filename is new */
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

inoptr srch_dir(inoptr wd, uint8_t *compname)
{
    uint_fast8_t curentry;
    blkno_t curblock;
    struct blkbuf *buf;
    struct direct *d;
    int nblocks;
    uint16_t inum;

    i_lock(wd);

    nblocks = inode_blocks(wd);

    for(curblock=0; curblock < nblocks; ++curblock) {
        buf = bread(wd->c_dev, bmap(wd, curblock, 1), 0);
        if (buf == NULL)
            break;
        for(curentry = 0; curentry < (BLKSIZE / DIR_LEN); ++curentry) {
            d = blkptr(buf, curentry * DIR_LEN, DIR_LEN);
            if(namecomp(compname, d->d_name)) {
                inum = d->d_ino;
                brelse(buf);
                i_unlock(wd);
                return i_open(wd->c_dev, inum);
            }
        }
        brelse(buf);
    }
    i_unlock(wd);
    return NULLINODE;
}


/* Srch_mt sees if the given inode is a mount point. If so it
 * dereferences it, and references and returns a pointer to the
 * root of the mounted filesystem.
 */

inoptr srch_mt(inoptr ino)
{
    register uint_fast8_t j;
    register struct mount *m = &fs_tab[0];

    for(j=0; j < NMOUNTS; ++j){
        if(m->m_dev != NO_DEVICE &&  m->m_mntpt == ino) {
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
 *
 * Once we support sleeping on bigger boxes during I/O we will need
 * a lock (superblock lock perhaps) to cover allocation of blocks and
 * inodes.
 */

inoptr i_open(uint16_t dev, uint16_t ino)
{
    regptr inoptr nindex;
    regptr inoptr j;
    struct mount *m;
    bool isnew = false;

    validchk(dev, PANIC_IOPEN);

    if(!ino){        /* ino==0 means we want a new one */
        isnew = true;
        ino = i_alloc(dev);
        if(!ino) {
            udata.u_error = ENOSPC;
            return NULLINODE;
        }
    }

    m = fs_tab_get(dev);

    /* Maybe make this DEBUG only eventually - the fs_tab_get cost
       is higher than ideal */
    if(ino < ROOTINODE || ino >= (m->m_fs.s_isize - 2) * INO_PER_BLOCK) {
        kputs("i_open: bad inode number\n");
        return NULLINODE;
    }

    nindex = NULLINODE;
    for(j = i_tab; j<i_tab + ITABSIZE; j++){
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

    if (breadi(dev, ino, &nindex->c_node))
        return NULLINODE;

    nindex->c_dev = dev;
    nindex->c_num = ino;
    nindex->c_super = m - fs_tab;
    nindex->c_magic = CMAGIC;
    nindex->c_flags = (m->m_flags & MS_RDONLY) ? CRDONLY : 0;
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

bool emptydir(inoptr wd)
{
    struct direct curentry;

    i_islocked(wd);

    udata.u_offset =  2 * DIR_LEN;	/* . .. ignored */

    do
    {
        udata.u_count = DIR_LEN;
        udata.u_base  = (uint8_t *)&curentry;
        udata.u_sysio = true;
        readi(wd, 0);

        /* Read until EOF or name is found.  readi() advances udata.u_offset */
        if (*curentry.d_name)
            return false;
    } while(udata.u_done == DIR_LEN);

    return true;
}


/* Ch_link modifies or makes a new entry in the directory for the name
 * and inode pointer given. The directory is searched for oldname.  When
 * found, it is changed to newname, and it inode # is that of *nindex.
 * A oldname of "" matches a unused slot, and a nindex of NULLINODE
 * means an inode # of 0.  A return status of 0 means there was no
 * space left in the filesystem, or a non-empty oldname was not found,
 * or the user did not have write permission.
 */

bool ch_link(inoptr wd, uint8_t *oldname, uint8_t *newname, inoptr nindex)
{
    struct direct curentry;
    int i;

    i_islocked(wd);

    if (wd->c_flags & CRDONLY) {
        udata.u_error = EROFS;
        return false;
    }
    /* FIXME: for modern style permissions we should also check whether
       wd has the sticky bit set and if so require ownership or root */
    if(!(getperm(wd) & OTH_WR))
    {
        udata.u_error = EACCES;
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
        udata.u_base  =(uint8_t *)&curentry;
        udata.u_sysio = true;
        readi(wd, 0);

        /* Read until EOF or name is found.  readi() advances udata.u_offset */
        if(udata.u_done == 0 || namecomp(oldname, curentry.d_name))
            break;
    }

    if(udata.u_done == 0 && *oldname) {
        udata.u_error = ENOENT;
        return false;                  /* Entry not found */
    }

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
    if(udata.u_done){
        udata.u_offset -= DIR_LEN;
    }

    udata.u_count = DIR_LEN;
    udata.u_base  = (unsigned char*)&curentry;
    udata.u_sysio = true;
    writei(wd, 0);

    if(udata.u_error)
        return false;

    setftime(wd, A_TIME|M_TIME|C_TIME);     /* Sets CDIRTY */

    /* Update file length to next block */
    if(BLKOFF(wd->c_node.i_size))
        wd->c_node.i_size += BLKSIZE - BLKOFF(wd->c_node.i_size);

    return true; // success
}

/* Namecomp compares two strings to see if they are the same file name.
 * It stops at FILENAME_LEN chars or a null or a slash. It returns 0 for difference.
 *
 * TODO: This generates crap code on most compilers so we probably ought to
 * turn it into platform asm code.
 */
bool namecomp(uint8_t *n1, uint8_t *n2) // return true if n1 == n2
{
    uint_fast8_t n; // do we have enough variables called n?

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
 *
 * Returns the new inode locked so nobody can access it before ready. We need
 * to think hard about newfile taking a callback to fix up the ino struct so
 * it's cleaner ???
 */

inoptr newfile(inoptr pino, uint8_t *name)
{
    regptr inoptr nindex;
    uint_fast8_t j;

    /* No parent? */
    if (!pino) {
        udata.u_error = ENXIO;
        goto nogood;
    }

    /* We check getperm before CRDONLY because if you reverse these two
       it breaks gcc 68hc11 3.4 */
    if (!(getperm(pino) & OTH_WR)) {
        udata.u_error = EPERM;
        goto nogood;
    }

    /* First see if parent is writeable */
    if (pino->c_flags & CRDONLY) {
        udata.u_error = EROFS;
        goto nogood;
    }

    if (!(nindex = i_open(pino->c_dev, 0))) {
        udata.u_error = ENFILE;
        goto nogood;
    }

    i_lock(pino);	/* Lock in tree order */
    i_lock(ino);
    /* This does not implement BSD style "sticky" groups */
    nindex->c_node.i_uid = udata.u_euid;
    nindex->c_node.i_gid = udata.u_egid;

    nindex->c_node.i_mode = F_REG;   /* For the time being */
    nindex->c_node.i_nlink = 1;
    nindex->c_node.i_size = 0;
    for (j = 0; j < 20; j++) {
        nindex->c_node.i_addr[j] = 0;
    }
    wr_inode(nindex);
    if (!ch_link(pino, (uint8_t *)"", name, nindex)) {
        i_deref(nindex);
	/* ch_link sets udata.u_error */
        goto nogood;
    }
    i_unlock_deref(pino);
    return nindex;

nogood:
    i_unlock_deref(pino);
    return NULLINODE;
}


/* Check the given device number, and return its address in the mount
 * table.  Also time-stamp the superblock of dev, and mark it modified.
 * Used when freeing and allocating blocks and inodes.
 */

fsptr getdev(uint16_t dev)
{
    regptr struct mount *mnt;
    time_t t;

    mnt = fs_tab_get(dev);

    if (!mnt || mnt->m_fs.s_mounted == 0) {
        panic(PANIC_GD_BAD);
        /* Return needed to persuade SDCC all is ok */
        return NULL;
    }
    if (!(mnt->m_flags & MS_RDONLY)) {
        rdtime(&t);
        mnt->m_fs.s_time = t.low;
        mnt->m_fs.s_timeh = t.high;
        mnt->m_fs.s_fmod = FMOD_DIRTY;
    }
    return &mnt->m_fs;
}


/* Returns true if the magic number of a superblock is corrupt. */

bool inline baddev(fsptr dev)
{
    return(dev->s_mounted != SMOUNTED);
}


/* I_alloc finds an unused inode number, and returns it, or 0
 * if there are no more inodes available.
 *
 * This will need to happen under the superblock lock once we do sleeping
 */

uint16_t i_alloc(uint16_t devno)
{
    staticfast fsptr dev;
    staticfast blkno_t blk;
    struct blkbuf *buf;
    regptr struct dinode *di;
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

    sync();           /* Make on-disk inodes consistent */
    k = 0;
    for(blk = 2; blk < dev->s_isize; blk++) {
        buf = bread(devno, blk, 0);
        if (buf == NULL)
            goto corrupt;
        for(j=0; j < INO_PER_BLOCK; j++) {
            /* Optimisation: add offsetof and use that to reduce blkptr range */
            di = blkptr(buf, sizeof(struct dinode) * j, sizeof(struct dinode));
            if(!(di->i_mode || di->i_nlink))
                dev->s_inode[k++] = INO_PER_BLOCK * (blk - 2) + j;
            if(k == FILESYS_TABSIZE) {
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
 *
 * This will need to happen under the superblock lock once we do sleeping
 */

void i_free(uint16_t devno, uint16_t ino)
{
    fsptr dev;

    if(baddev(dev = getdev(devno)))
        return;

    if(ino < 2 || ino >=(dev->s_isize-2)*8)
        panic(PANIC_IFREE_BADI);

    ++dev->s_tinode;
    if(dev->s_ninode < FILESYS_TABSIZE)
        dev->s_inode[dev->s_ninode++] = ino;
}


/* Blk_alloc is given a device number, and allocates an unused block
 * from it. A returned block number of zero means no more blocks.
 *
 * This will need to happen under the superblock lock once we do sleeping
 */

blkno_t blk_alloc(uint16_t devno)
{
    fsptr dev;
    blkno_t newno;
    struct blkbuf *buf;

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
        buf = bread(devno, newno, 0);
        if (buf == NULL)
            goto corrupt;
        blktok(&dev->s_nfree, buf, 0,
            sizeof(int) + FILESYS_TABSIZE * sizeof(blkno_t));
        /* This assumes no padding: this is an UZI era assumption */
        brelse(buf);
    }

    validblk(devno, newno);

    if(!dev->s_tfree)
        goto corrupt;
    --dev->s_tfree;

   /*
    * FIXME: When we implement the rest of the bigger block size fs support
    * this routine is responsible for zeroing the entire extent not just the
    * BLKSIZE byte block
    */
    /* Zero out the new block */
    buf = bread(devno, newno, 2);
    if (buf == NULL)
        goto corrupt;
    blkzero(buf);
    bawrite(buf);
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
 *
 * This will need to happen under the superblock lock once we do sleeping
 */

void blk_free(uint16_t devno, blkno_t blk)
{
    fsptr dev;
    struct blkbuf *buf;

    if(!blk)
        return;

    if(baddev(dev = getdev(devno)))
        return;

    validblk(devno, blk);

    if(dev->s_nfree == FILESYS_TABSIZE) {
        buf = bread(devno, blk, 1);
        if (buf) {
            /* nfree must directly preceed the blocks and without padding. That's
               the assumption UZI always had */
            blkfromk(&dev->s_nfree, buf, 0, sizeof(int) + 50 * sizeof(blkno_t));
            bawrite(buf);
            dev->s_nfree = 0;
        } else
            dev->s_mounted = 1;
    }

    ++dev->s_tfree;
    dev->s_free[(dev->s_nfree)++] = blk;
}


/* Oft_alloc and oft_deref allocate and dereference(and possibly free)
 * entries in the open file table.
 */

int_fast8_t oft_alloc(void)
{
    uint_fast8_t j;

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

/*
 *	To minimise storage we don't track exclusive locks explicitly. We know
 *	that if we are dropping an exclusive lock then we must be the owner,
 *	and if we are dropping a lock that is not exclusive we must own one of
 *	the non exclusive locks.
 */
void deflock(regptr struct oft *ofptr)
{
    inoptr i = ofptr->o_inode;
    uint_fast8_t c = i->c_flags & CFLOCK;

    if (ofptr->o_access & O_FLOCK) {
        if (c == CFLEX)
            c = 0;
        else
            c--;
        i->c_flags = (i->c_flags & ~CFLOCK) | c;
        wakeup(&i->c_flags);
    }
}

/*
 *	Drop a reference in the open file table. If this is the last reference
 *	from a user file table then drop any file locks, dereference the inode
 *	and mark empty
 */
void oft_deref(uint_fast8_t of)
{
    struct oft *ofptr;

    ofptr = of_tab + of;
    if(!(--ofptr->o_refs) && ofptr->o_inode) {
        deflock(ofptr);
        i_deref(ofptr->o_inode);
        ofptr->o_inode = NULLINODE;
    }
}


/* Uf_alloc finds an unused slot in the user file table.*/

int_fast8_t uf_alloc_n(uint_fast8_t base)
{
    uint_fast8_t j;

    for(j=base; j < UFTSIZE ; ++j) {
        if(udata.u_files[j] == NO_FILE) {
            return j;
        }
    }
    udata.u_error = EMFILE;
    return -1;
}


int_fast8_t uf_alloc(void)
{
    return uf_alloc_n(0);
}


/* I_deref decreases the reference count of an inode, and frees it from
 * the table if there are no more references to it.  If it also has no
 * links, the inode itself and its blocks(if not a device) is freed.
 */

void i_deref(regptr inoptr ino)
{
    uint_fast8_t mode = getmode(ino);

    magic(ino);

    if(!ino->c_refs)
        panic(PANIC_INODE_FREED);

    if (mode == MODE_R(F_PIPE))
        wakeup((uint8_t *)ino);

    /* If the inode has no links and no refs, it must have
       its blocks freed. */

    if(!(--ino->c_refs || ino->c_node.i_nlink))
        /*
           SN (mcy)
           */
        if (mode == MODE_R(F_REG) || mode == MODE_R(F_DIR) || mode == MODE_R(F_PIPE))
            f_trunc(ino);

    /* If the inode was modified, we must write it to disk. */
    if(!(ino->c_refs) && (ino->c_flags & CDIRTY))
    {
        if(!(ino->c_node.i_nlink))
        {
            ino->c_node.i_mode = 0;
            i_free(ino->c_dev, ino->c_num);
        }
        wr_inode(ino);
    }
}

void corrupt_fs(uint16_t devno)
{
    struct mount *mnt = fs_tab_get(devno);
    mnt->m_fs.s_mounted = 1;
    kputs("filesystem corrupt.\n");
}
/* Wr_inode writes out the given inode in the inode table out to disk,
 * and resets its dirty bit.
 */

void wr_inode(inoptr ino)
{
    struct blkbuf *buf;
    blkno_t blkno;

    magic(ino);

    if (bwritei(ino))
        corrupt_fs(ino->c_dev);
    else
        ino->c_flags &= ~CDIRTY;
}


/* isdevice(ino) returns true if ino points to a device */
bool isdevice(inoptr ino)
{
    return !!(ino->c_node.i_mode & F_CDEV);
}


/* This returns the device number of an inode representing a device */
uint16_t devnum(inoptr ino)
{
    return (uint16_t)ino->c_node.i_addr[0];
}


/* F_trunc frees all the blocks associated with the file, if it
 * is a disk file.
 */
int f_trunc(regptr inoptr ino)
{
    uint16_t dev;
    int_fast8_t j;

    if (ino->c_flags & CRDONLY) {
        udata.u_error = EROFS;
        return -1;
    }
    dev = ino->c_dev;

    /* First deallocate the double indirect blocks */
    freeblk(dev, ino->c_node.i_addr[19], 2);

    /* Also deallocate the indirect blocks */
    freeblk(dev, ino->c_node.i_addr[18], 1);

    /* Finally, free the direct blocks */
    for(j=17; j >= 0; --j)
        freeblk(dev, ino->c_node.i_addr[j], 0);

    memset((uint8_t *)ino->c_node.i_addr, 0, sizeof(ino->c_node.i_addr));

    ino->c_flags |= CDIRTY;
    ino->c_node.i_size = 0;
    return 0;
}

/* Companion function to f_trunc().

   This is the one case where we can't hide the difference between an internal
   and external buffer cache cleanly. The external one has a somewhat higher
   overhead (we could mitigate it by batching perhaps) and also size.

   This is annoying and it would be nice one day to find a clean solution */

#ifdef CONFIG_BLKBUF_EXTERNAL
void freeblk(uint16_t dev, blkno_t blk, uint_fast8_t level)
{
    struct blkbuf *buf;
    regptr blkno_t *bn;
    int16_t j;

    if(!blk)
        return;

    if(level){
        buf = bread(dev, blk, 0);
        if (buf == NULL) {
            corrupt_fs(dev);
            return;
        }
        for(j = BLKSIZE / 2 - 1; j >= 0; --j) {
            blktok(&bn, buf, j * sizeof(blkno_t), sizeof(blkno_t));
            freeblk(dev, bn[j], level-1);
        }
        brelse(buf);
    }
#ifdef CONFIG_TRIM
    d_ioctl(dev, HDIO_TRIM, (void*)&blk);
#endif
    blk_free(dev, blk);
}

#else

void freeblk(uint16_t dev, blkno_t blk, uint_fast8_t level)
{
    struct blkbuf *buf;
    regptr blkno_t *bn;
    int16_t j;

    if(!blk)
        return;

    if(level){
        buf = bread(dev, blk, 0);
        if (buf == NULL) {
            corrupt_fs(dev);
            return;
        }
        bn = blkptr(buf, 0, BLKSIZE);
        for(j = BLKSIZE / 2 - 1; j >= 0; --j)
            freeblk(dev, bn[j], level-1);
        brelse(buf);
    }
#ifdef CONFIG_TRIM
    d_ioctl(dev, HDIO_TRIM, (void*)&blk);
#endif
    blk_free(dev, blk);
}
#endif

/* Validblk panics if the given block number is not a valid
 *  data block for the given device.
 */
void validblk(uint16_t dev, blkno_t num)
{
    struct mount *mnt;

    mnt = fs_tab_get(dev);

    if(mnt == NULL || mnt->m_fs.s_mounted == 0) {
        panic(PANIC_VALIDBLK_NM);
        return;
    }

    if(num < mnt->m_fs.s_isize || num >= mnt->m_fs.s_fsize)
        panic(PANIC_VALIDBLK_INV);
}


/* This returns the inode pointer associated with a user's file
 * descriptor, checking for valid data structures.
 */
inoptr getinode(uint_fast8_t uindex)
{
    uint_fast8_t oftindex;
    inoptr inoindex;

    if(uindex >= UFTSIZE || udata.u_files[uindex] == NO_FILE) {
        udata.u_error = EBADF;
        return NULLINODE;
    }

    oftindex = udata.u_files[uindex];

    if(oftindex >= OFTSIZE || oftindex == NO_FILE)
        panic(PANIC_GETINO_BADT);

    if((inoindex = of_tab[oftindex].o_inode) < i_tab || inoindex >= i_tab+ITABSIZE)
        panic(PANIC_GETINO_OFT);

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
#ifdef CONFIG_LEVEL_2
    /* BSD process groups */
    else if (in_group(ino->c_node.i_gid))
        mode >>= 3;
#endif

    return(mode & 07);
}


/* This sets the times of the given inode, according to the flags. */
void setftime(inoptr ino, uint_fast8_t flag)
{
    if (ino->c_flags & CRDONLY)
        return;

    /* If only ATIME is due an update then skip it for a noatime fs */
    if (flag == A_TIME && fs_tab[ino->c_super].m_flags & MS_NOATIME)
        return;

    ino->c_flags |= CDIRTY;

    if(flag & A_TIME)
        rdtime32(&(ino->c_node.i_atime));
    if(flag & M_TIME)
        rdtime32(&(ino->c_node.i_mtime));
    if(flag & C_TIME)
        rdtime32(&(ino->c_node.i_ctime));
}

uint8_t getmode(inoptr ino)
{
    /* Shifting by 9 (past permissions) might be more logical but
       8 happens to be cheap */
    return (ino->c_node.i_mode & F_MASK) >> 8;
}

static struct mount *newfstab(void)
{
    register struct mount *m = fs_tab;
    register uint_fast8_t i;
    for (i = 0; i < NMOUNTS; i++) {
        if (m->m_dev == NO_DEVICE)
            return m;
        m++;
    }
    return NULL;
}

struct mount *fs_tab_get(uint16_t dev)
{
    register struct mount *m = fs_tab;
    register uint_fast8_t i;
    for (i = 0; i < NMOUNTS; i++) {
        if (m->m_dev == dev)
            return m;
        m++;
    }
    return NULL;
}

/* Fmount places the given device in the mount table with mount point info. */
struct mount *fmount(uint16_t dev, inoptr ino, uint16_t flags)
{
    struct mount *m;
    regptr struct filesys *fp;
    bufptr buf;

    if(d_open(dev, 0) != 0)
        return NULL;    /* Bad device */

    m = newfstab();
    if (m == NULL) {
        udata.u_error = EMFILE;
        return NULL;	/* Table is full */
    }

    fp = &m->m_fs;

    /* Get the buffer with the superblock (block 1) */
    buf = bread(dev, 1, 0);
    if (buf == NULL)
        return NULL;
    blktok(fp, buf, 0, sizeof(struct filesys));
    brelse(buf);

#ifdef DEBUG
    kprintf("fp->s_mounted=0x%x, fp->s_isize=0x%x, fp->s_fsize=0x%x\n",
    fp->s_mounted, fp->s_isize, fp->s_fsize);
#endif

    /* See if there really is a filesystem on the device */
    if(fp->s_mounted != SMOUNTED  ||  fp->s_isize >= fp->s_fsize ||
        fp->s_shift > FS_MAX_SHIFT) {
        udata.u_error = EINVAL;
        return NULL;
    }

    if (fp->s_fmod == FMOD_DIRTY) {
        kputs("warning: mounting dirty file system, forcing r/o.\n");
        flags |= MS_RDONLY;
    }
    if (!(flags & MS_RDONLY))
        /* Dirty - and will write dirty mark back to media */
        fp->s_fmod = FMOD_DIRTY;
    else	/* Clean in memory, don't write it back to media */
        fp->s_fmod = FMOD_CLEAN;
    m->m_mntpt = ino;
    if(ino)
        ++ino->c_refs;
    m->m_flags = flags;
    /* Makes our entry findable */
    m->m_dev = dev;

    /* Mark the filesystem dirty on disk */
    sync();

    return m;
}


void magic(inoptr ino)
{
    if(ino->c_magic != CMAGIC)
        panic(PANIC_CORRUPTI);
}

/* This is a helper function used by _unlink and _rename; it doesn't really
 * belong here, but needs to be in common code as it's used from two different
 * syscall banks.
 *
 * FIXME: this could be more efficient if we remembered which directory offset
 * we found the node at lookup time
 */
arg_t unlinki(inoptr ino, inoptr pino, uint8_t *fname)
{
	if (getmode(ino) == MODE_R(F_DIR)) {
		udata.u_error = EISDIR;
		return -1;
	}

	/* Remove the directory entry (ch_link checks perms) */
	if (!ch_link(pino, fname, (uint8_t *)"", NULLINODE))
		return -1;

	/* Decrease the link count of the inode */
	if (!(ino->c_node.i_nlink--)) {
		ino->c_node.i_nlink += 2;
		kprintf("_unlink: bad nlink\n");
	}
	setftime(ino, C_TIME);
	return (0);
}

