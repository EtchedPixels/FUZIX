extern page_t swappage;

extern int pageread(uint16_t dev, blkno_t blkno, usize_t nbytes,
                    paddr_t buf, page_t page);
extern int pagewrite(uint16_t dev, blkno_t blkno, usize_t nbytes,
		    paddr_t buf, page_t page);
extern void pagefile_add_blocks(unsigned blocks);
