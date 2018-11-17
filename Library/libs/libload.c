#include <stdint.h>
#include <sys/library.h>
#include <fcntl.h>
#include <unistd.h>

/* FIXME: this assumes simple 16bit relocations. We will need to make this
   arch specific in some cases */
int libload(struct library *lib, uint8_t *addr)
{
	uint8_t *p;
	uint8_t *r = addr - 1;
	uint16_t rel;

	rel = (uint16_t)addr;

	if (rel & 0xFF)
		return -1;
	rel >>= 8;

	if (lseek(lib->fd, 512, SEEK_SET) < 0)
		return -1;
	if (read(lib->fd, addr, lib->size) != lib->size)
		return -1;
	p = lib->reloc;
	while(*p) {
		if (*p == 255) {
			p++;
			r += 254;
			continue;
		}
		r += *p++;
		*r += rel;
	}
	return 0;
}
