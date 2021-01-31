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

#ifndef DHARA_NAND_H_
#define DHARA_NAND_H_

#include <stdint.h>
#include <stddef.h>
#include "error.h"

/* Each page in a NAND device is indexed, starting at 0. It's required
 * that there be a power-of-two number of pages in a eraseblock, so you can
 * view a page number is being a concatenation (in binary) of a block
 * number and the number of a page within a block.
 */
typedef uint32_t dhara_page_t;

/* Blocks are also indexed, starting at 0. */
typedef uint32_t dhara_block_t;

/* Each NAND chip must be represented by one of these structures. It's
 * intended that this structure be embedded in a larger structure for
 * context.
 *
 * The functions declared below are not implemented -- they must be
 * provided and satisfy the documented conditions.
 */
struct dhara_nand {
	/* Base-2 logarithm of the page size. If your device supports
	 * partial programming, you may want to subdivide the actual
	 * pages into separate ECC-correctable regions and present those
	 * as pages.
	 */
	uint8_t		log2_page_size;

	/* Base-2 logarithm of the number of pages within an eraseblock */
	uint8_t		log2_ppb;

	/* Total number of eraseblocks */
	unsigned int	num_blocks;
};

/* Is the given block bad? */
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b);

/* Mark bad the given block (or attempt to). No return value is
 * required, because there's nothing that can be done in response.
 */
void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b);

/* Erase the given block. This function should return 0 on success or -1
 * on failure.
 *
 * The status reported by the chip should be checked. If an erase
 * operation fails, return -1 and set err to E_BAD_BLOCK.
 */
int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b,
		     dhara_error_t *err);

/* Program the given page. The data pointer is a pointer to an entire
 * page ((1 << log2_page_size) bytes). The operation status should be
 * checked. If the operation fails, return -1 and set err to
 * E_BAD_BLOCK.
 *
 * Pages will be programmed sequentially within a block, and will not be
 * reprogrammed.
 */
int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
		    const uint8_t *data,
		    dhara_error_t *err);

/* Check that the given page is erased */
int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p);

/* Read a portion of a page. ECC must be handled by the NAND
 * implementation. Returns 0 on sucess or -1 if an error occurs. If an
 * uncorrectable ECC error occurs, return -1 and set err to E_ECC.
 */
int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
		    size_t offset, size_t length,
		    uint8_t *data,
		    dhara_error_t *err);

/* Read a page from one location and reprogram it in another location.
 * This might be done using the chip's internal buffers, but it must use
 * ECC.
 */
int dhara_nand_copy(const struct dhara_nand *n,
		    dhara_page_t src, dhara_page_t dst,
		    dhara_error_t *err);

#endif
