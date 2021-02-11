/* Dhara - NAND flash management layer
 * Copyright (C) 2013 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DHARA_MAP_H_
#define DHARA_MAP_H_

#include "journal.h"

/* The map is a journal indexing format. It maps virtual sectors to
 * pages of data in flash memory.
 */
typedef uint32_t dhara_sector_t;

/* This sector value is reserved */
#define DHARA_SECTOR_NONE	0xffffffff

struct dhara_map {
	struct dhara_journal	journal;

	uint8_t			gc_ratio;
	dhara_sector_t		count;
};

/* Initialize a map. You need to supply a buffer for page metadata, and
 * a garbage collection ratio. This is the ratio of garbage collection
 * operations to real writes when automatic collection is active.
 *
 * Smaller values lead to faster and more predictable IO, at the
 * expense of capacity. You should always initialize the same chip with
 * the same garbage collection ratio.
 */
void dhara_map_init(struct dhara_map *m, const struct dhara_nand *n,
		    uint8_t *page_buf, uint8_t gc_ratio);

/* Recover stored state, if possible. If there is no valid stored state
 * on the chip, -1 is returned, and an empty map is initialized.
 */
int dhara_map_resume(struct dhara_map *m, dhara_error_t *err);

/* Clear the map (delete all sectors). */
void dhara_map_clear(struct dhara_map *m);

/* Obtain the maximum capacity of the map. */
dhara_sector_t dhara_map_capacity(const struct dhara_map *m);

/* Obtain the current number of allocated sectors. */
static inline dhara_sector_t dhara_map_size(const struct dhara_map *m)
{
	return m->count;
}

/* Find the physical page which holds the current data for this sector.
 * Returns 0 on success or -1 if an error occurs. If the sector doesn't
 * exist, the error is E_NOT_FOUND.
 */
int dhara_map_find(struct dhara_map *m, dhara_sector_t s,
		   dhara_page_t *loc, dhara_error_t *err);

/* Read from the given logical sector. If the sector is unmapped, a
 * blank page (0xff) will be returned.
 */
int dhara_map_read(struct dhara_map *m, dhara_sector_t s,
		   uint8_t *data, dhara_error_t *err);

/* Write data to a logical sector. */
int dhara_map_write(struct dhara_map *m, dhara_sector_t s,
		    const uint8_t *data, dhara_error_t *err);

/* Copy any flash page to a logical sector. */
int dhara_map_copy_page(struct dhara_map *m, dhara_page_t src,
			dhara_sector_t dst, dhara_error_t *err);

/* Copy one sector to another. If the source sector is unmapped, the
 * destination sector will be trimmed.
 */
int dhara_map_copy_sector(struct dhara_map *m, dhara_sector_t src,
			  dhara_sector_t dst, dhara_error_t *err);

/* Delete a logical sector. You don't necessarily need to do this, but
 * it's a useful hint if you no longer require the sector's data to be
 * kept.
 *
 * If order is non-zero, it specifies that all sectors in the
 * (2**order)-aligned group of s are to be deleted.
 */
int dhara_map_trim(struct dhara_map *m, dhara_sector_t s,
		   dhara_error_t *err);

/* Synchronize the map. Once this returns successfully, all changes to
 * date are persistent and durable. Conversely, there is no guarantee
 * that unsynchronized changes will be persistent.
 */
int dhara_map_sync(struct dhara_map *m, dhara_error_t *err);

/* Perform one garbage collection step. You can do this whenever you
 * like, but it's not necessary -- garbage collection happens
 * automatically and is interleaved with other operations.
 */
int dhara_map_gc(struct dhara_map *m, dhara_error_t *err);

#endif
