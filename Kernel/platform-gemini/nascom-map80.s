;
;	MAP-80 is more flexible than the standard setup
;
;	Port 0xFE starts at 0x00 bits
;
;	7:	32K page mode (64K if clear)
;	6:	1 upper 32K fixed 0 lower 32K fixed in 32K mode
;	1-5:	Page number  (bit 5 is ignored in 64K mode as it's the
;		low/high bits
;	0:	1 = select upper 0 = select lower half of page 0 in fixed area
;
;	256K per card, links allow max of 2MB total
;
;
;	Can also be fronted by the GM813 instead for 512K of 4K pages
;
;	Can in theory mix with classic FF paging by mapping non existant
;	MAP80 pages then mapping an FF page and vice versa (no existant FF
;	page) - ugly.
;


