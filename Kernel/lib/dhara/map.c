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

#include <string.h>
#include "bytes.h"
#include "map.h"

#define DHARA_RADIX_DEPTH	(sizeof(dhara_sector_t) << 3)

static inline dhara_sector_t d_bit(int depth)
{
	return ((dhara_sector_t)1) << (DHARA_RADIX_DEPTH - depth - 1);
}

/************************************************************************
 * Metadata/cookie layout
 */

static inline void ck_set_count(uint8_t *cookie, dhara_sector_t count)
{
	dhara_w32(cookie, count);
}

static inline dhara_sector_t ck_get_count(const uint8_t *cookie)
{
	return dhara_r32(cookie);
}

static inline void meta_clear(uint8_t *meta)
{
	memset(meta, 0xff, DHARA_META_SIZE);
}

static inline dhara_sector_t meta_get_id(const uint8_t *meta)
{
	return dhara_r32(meta);
}

static inline void meta_set_id(uint8_t *meta, dhara_sector_t id)
{
	dhara_w32(meta, id);
}

static inline dhara_page_t meta_get_alt(const uint8_t *meta, int level)
{
	return dhara_r32(meta + 4 + (level << 2));
}

static inline void meta_set_alt(uint8_t *meta, int level, dhara_page_t alt)
{
	dhara_w32(meta + 4 + (level << 2), alt);
}

/************************************************************************
 * Public interface
 */

void dhara_map_init(struct dhara_map *m, const struct dhara_nand *n,
		    uint8_t *page_buf, uint8_t gc_ratio)
{
	if (!gc_ratio)
		gc_ratio = 1;

	dhara_journal_init(&m->journal, n, page_buf);
	m->gc_ratio = gc_ratio;
}

int dhara_map_resume(struct dhara_map *m, dhara_error_t *err)
{
	if (dhara_journal_resume(&m->journal, err) < 0) {
		m->count = 0;
		return -1;
	}

	m->count = ck_get_count(dhara_journal_cookie(&m->journal));
	return 0;
}

void dhara_map_clear(struct dhara_map *m)
{
	if (m->count) {
		m->count = 0;
		dhara_journal_clear(&m->journal);
	}
}

dhara_sector_t dhara_map_capacity(const struct dhara_map *m)
{
	const dhara_sector_t cap = dhara_journal_capacity(&m->journal);
	const dhara_sector_t reserve = cap / (m->gc_ratio + 1);
	const dhara_sector_t safety_margin =
		DHARA_MAX_RETRIES << m->journal.nand->log2_ppb;

	if (reserve + safety_margin >= cap)
		return 0;

	return cap - reserve - safety_margin;
}

/* Trace the path from the root to the given sector, emitting
 * alt-pointers and alt-full bits in the given metadata buffer. This
 * also returns the physical page containing the given sector, if it
 * exists.
 *
 * If the page can't be found, a suitable path will be constructed
 * (containing PAGE_NONE alt-pointers), and DHARA_E_NOT_FOUND will be
 * returned.
 */
static int trace_path(struct dhara_map *m, dhara_sector_t target,
		      dhara_page_t *loc, uint8_t *new_meta,
		      dhara_error_t *err)
{
	uint8_t meta[DHARA_META_SIZE];
	int depth = 0;
	dhara_page_t p = dhara_journal_root(&m->journal);

	if (new_meta)
		meta_set_id(new_meta, target);

	if (p == DHARA_PAGE_NONE)
		goto not_found;

	if (dhara_journal_read_meta(&m->journal, p, meta, err) < 0)
		return -1;

	while (depth < DHARA_RADIX_DEPTH) {
		const dhara_sector_t id = meta_get_id(meta);

		if (id == DHARA_SECTOR_NONE)
			goto not_found;

		if ((target ^ id) & d_bit(depth)) {
			if (new_meta)
				meta_set_alt(new_meta, depth, p);

			p = meta_get_alt(meta, depth);
			if (p == DHARA_PAGE_NONE) {
				depth++;
				goto not_found;
			}

			if (dhara_journal_read_meta(&m->journal, p,
						    meta, err) < 0)
				return -1;
		} else {
			if (new_meta)
				meta_set_alt(new_meta, depth,
					meta_get_alt(meta, depth));
		}

		depth++;
	}

	if (loc)
		*loc = p;

	return 0;

not_found:
	if (new_meta) {
		while (depth < DHARA_RADIX_DEPTH)
			meta_set_alt(new_meta, depth++, DHARA_SECTOR_NONE);
	}

	dhara_set_error(err, DHARA_E_NOT_FOUND);
	return -1;
}

int dhara_map_find(struct dhara_map *m, dhara_sector_t target,
		   dhara_page_t *loc, dhara_error_t *err)
{
	return trace_path(m, target, loc, NULL, err);
}

int dhara_map_read(struct dhara_map *m, dhara_sector_t s,
		   uint8_t *data, dhara_error_t *err)
{
	const struct dhara_nand *n = m->journal.nand;
	dhara_error_t my_err;
	dhara_page_t p;

	if (dhara_map_find(m, s, &p, &my_err) < 0) {
		if (my_err == DHARA_E_NOT_FOUND) {
			memset(data, 0xff, 1 << n->log2_page_size);
			return 0;
		}

		dhara_set_error(err, my_err);
		return -1;
	}

	return dhara_nand_read(n, p, 0, 1 << n->log2_page_size, data, err);
}

/* Check the given page. If it's garbage, do nothing. Otherwise, rewrite
 * it at the front of the map. Return raw errors from the journal (do
 * not perform recovery).
 */
static int raw_gc(struct dhara_map *m, dhara_page_t src,
		  dhara_error_t *err)
{
	dhara_sector_t target;
	dhara_page_t current;
	dhara_error_t my_err;
	uint8_t meta[DHARA_META_SIZE];

	if (dhara_journal_read_meta(&m->journal, src, meta, err) < 0)
		return -1;

	/* Is the page just filler/garbage? */
	target = meta_get_id(meta);
	if (target == DHARA_SECTOR_NONE)
		return 0;

	/* Find out where the sector once represented by this page
	 * currently resides (if anywhere).
	 */
	if (trace_path(m, target, &current, meta, &my_err) < 0) {
		if (my_err == DHARA_E_NOT_FOUND)
			return 0;

		dhara_set_error(err, my_err);
		return -1;
	}

	/* Is this page still the most current representative? If not,
	 * do nothing.
	 */
	if (current != src)
		return 0;

	/* Rewrite it at the front of the journal with updated metadata */
	ck_set_count(dhara_journal_cookie(&m->journal), m->count);
	if (dhara_journal_copy(&m->journal, src, meta, err) < 0)
		return -1;

	return 0;
}

static int pad_queue(struct dhara_map *m, dhara_error_t *err)
{
	dhara_page_t p = dhara_journal_root(&m->journal);
	uint8_t root_meta[DHARA_META_SIZE];

	ck_set_count(dhara_journal_cookie(&m->journal), m->count);

	if (p == DHARA_PAGE_NONE)
		return dhara_journal_enqueue(&m->journal, NULL, NULL, err);

	if (dhara_journal_read_meta(&m->journal, p, root_meta, err) < 0)
		return -1;

	return dhara_journal_copy(&m->journal, p, root_meta, err);
}

/* Attempt to recover the journal */
static int try_recover(struct dhara_map *m, dhara_error_t cause,
		       dhara_error_t *err)
{
	int restart_count = 0;

	if (cause != DHARA_E_RECOVER) {
		dhara_set_error(err, cause);
		return -1;
	}

	while (dhara_journal_in_recovery(&m->journal)) {
		dhara_page_t p = dhara_journal_next_recoverable(&m->journal);
		dhara_error_t my_err;
		int ret;

		if (p == DHARA_PAGE_NONE)
			ret = pad_queue(m, &my_err);
		else
			ret = raw_gc(m, p, &my_err);

		if (ret < 0) {
			if (my_err != DHARA_E_RECOVER) {
				dhara_set_error(err, my_err);
				return -1;
			}

			if (restart_count >= DHARA_MAX_RETRIES) {
				dhara_set_error(err, DHARA_E_TOO_BAD);
				return -1;
			}

			restart_count++;
		}
	}

	return 0;
}

static int auto_gc(struct dhara_map *m, dhara_error_t *err)
{
	int i;

	if (dhara_journal_size(&m->journal) < dhara_map_capacity(m))
		return 0;

	for (i = 0; i < m->gc_ratio; i++)
		if (dhara_map_gc(m, err) < 0)
			return -1;

	return 0;
}

static int prepare_write(struct dhara_map *m, dhara_sector_t dst,
			 uint8_t *meta, dhara_error_t *err)
{
	dhara_error_t my_err;

	if (auto_gc(m, err) < 0)
		return -1;

	if (trace_path(m, dst, NULL, meta, &my_err) < 0) {
		if (my_err != DHARA_E_NOT_FOUND) {
			dhara_set_error(err, my_err);
			return -1;
		}

		if (m->count >= dhara_map_capacity(m)) {
			dhara_set_error(err, DHARA_E_MAP_FULL);
			return -1;
		}

		m->count++;
	}

	ck_set_count(dhara_journal_cookie(&m->journal), m->count);
	return 0;
}

int dhara_map_write(struct dhara_map *m, dhara_sector_t dst,
		    const uint8_t *data, dhara_error_t *err)
{
	for (;;) {
		uint8_t meta[DHARA_META_SIZE];
		dhara_error_t my_err;
		const dhara_sector_t old_count = m->count;

		if (prepare_write(m, dst, meta, err) < 0)
			return -1;

		if (!dhara_journal_enqueue(&m->journal, data, meta, &my_err))
			break;

		m->count = old_count;

		if (try_recover(m, my_err, err) < 0)
			return -1;
	}

	return 0;
}

int dhara_map_copy_page(struct dhara_map *m, dhara_page_t src,
			dhara_sector_t dst, dhara_error_t *err)
{
	for (;;) {
		uint8_t meta[DHARA_META_SIZE];
		dhara_error_t my_err;
		const dhara_sector_t old_count = m->count;

		if (prepare_write(m, dst, meta, err) < 0)
			return -1;

		if (!dhara_journal_copy(&m->journal, src, meta, &my_err))
			break;

		m->count = old_count;

		if (try_recover(m, my_err, err) < 0)
			return -1;
	}

	return 0;
}

int dhara_map_copy_sector(struct dhara_map *m, dhara_sector_t src,
			  dhara_sector_t dst, dhara_error_t *err)
{
	dhara_error_t my_err;
	dhara_page_t p;

	if (dhara_map_find(m, src, &p, &my_err) < 0) {
		if (my_err == DHARA_E_NOT_FOUND)
			return dhara_map_trim(m, dst, err);

		dhara_set_error(err, my_err);
		return -1;
	}

	return dhara_map_copy_page(m, p, dst, err);
}

static int try_delete(struct dhara_map *m, dhara_sector_t s,
		      dhara_error_t *err)
{
	dhara_error_t my_err;
	uint8_t meta[DHARA_META_SIZE];
	dhara_page_t alt_page;
	uint8_t alt_meta[DHARA_META_SIZE];
	int level = DHARA_RADIX_DEPTH - 1;
	int i;

	if (trace_path(m, s, NULL, meta, &my_err) < 0) {
		if (my_err == DHARA_E_NOT_FOUND)
			return 0;

		dhara_set_error(err, my_err);
		return -1;
	}

	/* Select any of the closest cousins of this node which are
	 * subtrees of at least the requested order.
	 */
	while (level >= 0) {
		alt_page = meta_get_alt(meta, level);
		if (alt_page != DHARA_PAGE_NONE)
			break;
		level--;
	}

	/* Special case: deletion of last sector */
	if (level < 0) {
		m->count = 0;
		dhara_journal_clear(&m->journal);
		return 0;
	}

	/* Rewrite the cousin with an up-to-date path which doesn't
	 * point to the original node.
	 */
	if (dhara_journal_read_meta(&m->journal, alt_page, alt_meta, err) < 0)
		return -1;

	meta_set_id(meta, meta_get_id(alt_meta));

	meta_set_alt(meta, level, DHARA_PAGE_NONE);
	for (i = level + 1; i < DHARA_RADIX_DEPTH; i++)
		meta_set_alt(meta, i, meta_get_alt(alt_meta, i));

	meta_set_alt(meta, level, DHARA_PAGE_NONE);

	ck_set_count(dhara_journal_cookie(&m->journal), m->count - 1);
	if (dhara_journal_copy(&m->journal, alt_page, meta, err) < 0)
		return -1;

	m->count--;
	return 0;
}

int dhara_map_trim(struct dhara_map *m, dhara_sector_t s, dhara_error_t *err)
{
	for (;;) {
		dhara_error_t my_err;

		if (auto_gc(m, err) < 0)
			return -1;

		if (!try_delete(m, s, &my_err))
			break;

		if (try_recover(m, my_err, err) < 0)
			return -1;
	}

	return 0;
}

int dhara_map_sync(struct dhara_map *m, dhara_error_t *err)
{
	while (!dhara_journal_is_clean(&m->journal)) {
		dhara_page_t p = dhara_journal_peek(&m->journal);
		dhara_error_t my_err;
		int ret;

		if (p == DHARA_PAGE_NONE) {
			ret = pad_queue(m, &my_err);
		} else {
			ret = raw_gc(m, p, &my_err);
			dhara_journal_dequeue(&m->journal);
		}

		if ((ret < 0) && (try_recover(m, my_err, err) < 0))
			return -1;
	}

	return 0;
}

int dhara_map_gc(struct dhara_map *m, dhara_error_t *err)
{
	if (!m->count)
		return 0;

	for (;;) {
		dhara_page_t tail = dhara_journal_peek(&m->journal);
		dhara_error_t my_err;

		if (tail == DHARA_PAGE_NONE)
			break;

		if (!raw_gc(m, tail, &my_err)) {
			dhara_journal_dequeue(&m->journal);
			break;
		}

		if (try_recover(m, my_err, err) < 0)
			return -1;
	}

	return 0;
}
