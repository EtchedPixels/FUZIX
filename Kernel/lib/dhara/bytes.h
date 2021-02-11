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

#ifndef DHARA_BYTES_H_
#define DHARA_BYTES_H_

#include <stdint.h>

static inline uint16_t dhara_r16(const uint8_t *data)
{
	return ((uint16_t)data[0]) |
	       (((uint16_t)data[1]) << 8);
}

static inline void dhara_w16(uint8_t *data, uint16_t v)
{
	data[0] = v;
	data[1] = v >> 8;
}

static inline uint32_t dhara_r32(const uint8_t *data)
{
	return ((uint32_t)data[0]) |
	       (((uint32_t)data[1]) << 8) |
	       (((uint32_t)data[2]) << 16) |
	       (((uint32_t)data[3]) << 24);
}

static inline void dhara_w32(uint8_t *data, uint32_t v)
{
	data[0] = v;
	data[1] = v >> 8;
	data[2] = v >> 16;
	data[3] = v >> 24;
}

#endif
