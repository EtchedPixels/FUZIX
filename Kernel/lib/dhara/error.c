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

#include <stddef.h>
#include "error.h"

const char *dhara_strerror(dhara_error_t err)
{
	static const char *const messages[DHARA_E_MAX] = {
		[DHARA_E_NONE] = "No error",
		[DHARA_E_BAD_BLOCK] = "Bad page/eraseblock",
		[DHARA_E_ECC] = "ECC failure",
		[DHARA_E_TOO_BAD] = "Too many bad blocks",
		[DHARA_E_RECOVER] = "Journal recovery is required",
		[DHARA_E_JOURNAL_FULL] = "Journal is full",
		[DHARA_E_NOT_FOUND] = "No such sector",
		[DHARA_E_MAP_FULL] = "Sector map is full",
		[DHARA_E_CORRUPT_MAP] = "Sector map is corrupted"
	};
	const char *msg = NULL;

	if ((err >= 0) && (err < DHARA_E_MAX))
		msg = messages[err];

	if (msg)
		return msg;

	return "Unknown error";
}
