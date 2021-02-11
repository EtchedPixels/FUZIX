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

#ifndef DHARA_JOURNAL_H_
#define DHARA_JOURNAL_H_

#include <stdint.h>
#include "nand.h"

/* Number of bytes used by the journal checkpoint header. */
#define DHARA_HEADER_SIZE		16

/* Global metadata available for a higher layer. This metadata is
 * persistent once the journal reaches a checkpoint, and is restored on
 * startup.
 */
#define DHARA_COOKIE_SIZE		4

/* This is the size of the metadata slice which accompanies each written
 * page. This is independent of the underlying page/OOB size.
 */
#define DHARA_META_SIZE			132

/* When a block fails, or garbage is encountered, we try again on the
 * next block/checkpoint. We can do this up to the given number of
 * times.
 */
#define DHARA_MAX_RETRIES		8

/* This is a page number which can be used to represent "no such page".
 * It's guaranteed to never be a valid user page.
 */
#define DHARA_PAGE_NONE			((dhara_page_t)0xffffffff)

/* State flags */
#define DHARA_JOURNAL_F_DIRTY		0x01
#define DHARA_JOURNAL_F_BAD_META	0x02
#define DHARA_JOURNAL_F_RECOVERY	0x04
#define DHARA_JOURNAL_F_ENUM_DONE	0x08

/* The journal layer presents the NAND pages as a double-ended queue.
 * Pages, with associated metadata may be pushed onto the end of the
 * queue, and pages may be popped from the end.
 *
 * Block erase, metadata storage are handled automatically. Bad blocks
 * are handled by relocating data to the next available non-bad page in
 * the sequence.
 *
 * It's up to the user to ensure that the queue doesn't grow beyond the
 * capacity of the NAND chip, but helper functions are provided to
 * assist with this. If the head meets the tail, the journal will refuse
 * to enqueue more pages.
 */
struct dhara_journal {
	const struct dhara_nand		*nand;
	uint8_t				*page_buf;

	/* In the journal, user data is grouped into checkpoints of
	 * 2**log2_ppc contiguous aligned pages.
	 *
	 * The last page of each checkpoint contains the journal header
	 * and the metadata for the other pages in the period (the user
	 * pages).
	 */
	uint8_t				log2_ppc;

	/* Epoch counter. This is incremented whenever the journal head
	 * passes the end of the chip and wraps around.
	 */
	uint8_t				epoch;

	/* General purpose flags field */
	uint8_t				flags;

	/* Bad-block counters. bb_last is our best estimate of the
	 * number of bad blocks in the chip as a whole. bb_current is
	 * the number of bad blocks in all blocks before the current
	 * head.
	 */
	dhara_block_t			bb_current;
	dhara_block_t			bb_last;

	/* Log head and tail. The tail pointer points to the last user
	 * page in the log, and the head pointer points to the next free
	 * raw page. The root points to the last written user page.
	 */
	dhara_page_t			tail_sync;
	dhara_page_t			tail;
	dhara_page_t			head;

	/* This points to the last written user page in the journal */
	dhara_page_t			root;

	/* Recovery mode: recover_root points to the last valid user
	 * page in the block requiring recovery. recover_next points to
	 * the next user page needing recovery.
	 *
	 * If we had buffered metadata before recovery started, it will
	 * have been dumped to a free page, indicated by recover_meta.
	 * If this block later goes bad, we will have to defer bad-block
	 * marking until recovery is complete (F_BAD_META).
	 */
	dhara_page_t			recover_next;
	dhara_page_t			recover_root;
	dhara_page_t			recover_meta;
};

/* Initialize a journal. You must supply a pointer to a NAND chip
 * driver, and a single page buffer. This page buffer will be used
 * exclusively by the journal, but you are responsible for allocating
 * it, and freeing it (if necessary) at the end.
 *
 * No NAND operations are performed at this point.
 */
void dhara_journal_init(struct dhara_journal *j,
			const struct dhara_nand *n,
			uint8_t *page_buf);

/* Start up the journal -- search the NAND for the journal head, or
 * initialize a blank journal if one isn't found. Returns 0 on success
 * or -1 if a (fatal) error occurs.
 *
 * This operation is O(log N), where N is the number of pages in the
 * NAND chip. All other operations are O(1).
 *
 * If this operation fails, the journal will be reset to an empty state.
 */
int dhara_journal_resume(struct dhara_journal *j, dhara_error_t *err);

/* Obtain an upper bound on the number of user pages storable in the
 * journal.
 */
dhara_page_t dhara_journal_capacity(const struct dhara_journal *j);

/* Obtain an upper bound on the number of user pages consumed by the
 * journal.
 */
dhara_page_t dhara_journal_size(const struct dhara_journal *j);

/* Obtain a pointer to the cookie data */
static inline uint8_t *dhara_journal_cookie(const struct dhara_journal *j)
{
	return j->page_buf + DHARA_HEADER_SIZE;
}

/* Obtain the locations of the first and last pages in the journal.
 */
static inline dhara_page_t dhara_journal_root(const struct dhara_journal *j)
{
	return j->root;
}

/* Read metadata associated with a page. This assumes that the page
 * provided is a valid data page. The actual page data is read via the
 * normal NAND interface.
 */
int dhara_journal_read_meta(struct dhara_journal *j, dhara_page_t p,
			    uint8_t *buf, dhara_error_t *err);

/* Advance the tail to the next non-bad block and return the page that's
 * ready to read. If no page is ready, return DHARA_PAGE_NONE.
 */
dhara_page_t dhara_journal_peek(struct dhara_journal *j);

/* Remove the last page from the journal. This doesn't take permanent
 * effect until the next checkpoint.
 */
void dhara_journal_dequeue(struct dhara_journal *j);

/* Remove all pages form the journal. This doesn't take permanent effect
 * until the next checkpoint.
 */
void dhara_journal_clear(struct dhara_journal *j);

/* Append a page to the journal. Both raw page data and metadata must be
 * specified. The push operation is not persistent until a checkpoint is
 * reached.
 *
 * This operation may fail with the error code E_RECOVER. If this
 * occurs, the upper layer must complete the assisted recovery procedure
 * and then try again.
 *
 * This operation may be used as part of a recovery. If further errors
 * occur during recovery, E_RECOVER is returned, and the procedure must
 * be restarted.
 */
int dhara_journal_enqueue(struct dhara_journal *j,
			  const uint8_t *data, const uint8_t *meta,
			  dhara_error_t *err);

/* Copy an existing page to the front of the journal. New metadata must
 * be specified. This operation is not persistent until a checkpoint is
 * reached.
 *
 * This operation may fail with the error code E_RECOVER. If this
 * occurs, the upper layer must complete the assisted recovery procedure
 * and then try again.
 *
 * This operation may be used as part of a recovery. If further errors
 * occur during recovery, E_RECOVER is returned, and the procedure must
 * be restarted.
 */
int dhara_journal_copy(struct dhara_journal *j,
		       dhara_page_t p, const uint8_t *meta,
		       dhara_error_t *err);

/* Mark the journal dirty. */
static inline void dhara_journal_mark_dirty(struct dhara_journal *j)
{
	j->flags |= DHARA_JOURNAL_F_DIRTY;
}

/* Is the journal checkpointed? If true, then all pages enqueued are now
 * persistent.
 */
static inline int dhara_journal_is_clean(const struct dhara_journal *j)
{
	return !(j->flags & DHARA_JOURNAL_F_DIRTY);
}

/* If an operation returns E_RECOVER, you must begin the recovery
 * procedure. You must then:
 *
 *    - call dhara_journal_next_recoverable() to obtain the next block
 *      to be recovered (if any). If there are no blocks remaining to be
 *      recovered, DHARA_JOURNAL_PAGE_NONE is returned.
 *
 *    - proceed to the next checkpoint. Once the journal is clean,
 *      recovery will finish automatically.
 *
 * If any operation during recovery fails due to a bad block, E_RECOVER
 * is returned again, and recovery restarts. Do not add new data to the
 * journal (rewrites of recovered data are fine) until recovery is
 * complete.
 */
static inline int dhara_journal_in_recovery(const struct dhara_journal *j)
{
	return j->flags & DHARA_JOURNAL_F_RECOVERY;
}

dhara_page_t dhara_journal_next_recoverable(struct dhara_journal *j);

#endif
