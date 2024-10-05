#define MAXNAME		1024

#define	NAMELEN		16	/* 15 usable due to _ lead on C names */

struct name {
	char name[NAMELEN];
	uint16_t id;
	struct name *next;
};
