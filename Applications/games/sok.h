#define MAP_H	14
#define MAP_W	32

/* Disk align each sector for speed */
#define MAP_SEEK	512

struct level {
	uint8_t map[MAP_H][MAP_W];
	uint8_t py;
	uint8_t px;
	uint8_t done;
	uint8_t blocks;
};
