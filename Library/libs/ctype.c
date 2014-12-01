/*
 *  CTYPE.C	Character classification and conversion
 */
/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
#include <types.h>
#include <ctype.h>

#undef	toupper
#undef	tolower

unsigned char __ctype[256] = { __CT_c, __CT_c, __CT_c, __CT_c, /* 0x00..0x03 */
                               __CT_c, __CT_c, __CT_c, __CT_c, /* 0x04..0x07 */
                               __CT_c, __CT_c | __CT_s, __CT_c | __CT_s, __CT_c | __CT_s, /* 0x08..0x0B */
                               __CT_c | __CT_s, __CT_c | __CT_s, __CT_c, __CT_c, /* 0x0C..0x0F */
                               __CT_c, __CT_c, __CT_c, __CT_c, /* 0x10..0x13 */
                               __CT_c, __CT_c, __CT_c, __CT_c, /* 0x14..0x17 */
                               __CT_c, __CT_c, __CT_c, __CT_c, /* 0x18..0x1B */
                               __CT_c, __CT_c, __CT_c, __CT_c, /* 0x1C..0x1F */
                               __CT_s, __CT_p, __CT_p, __CT_p, /* 0x20..0x23 */
                               __CT_p, __CT_p, __CT_p, __CT_p, /* 0x24..0x27 */
                               __CT_p, __CT_p, __CT_p, __CT_p, /* 0x28..0x2B */
                               __CT_p, __CT_p, __CT_p, __CT_p, /* 0x2C..0x2F */
                               __CT_d | __CT_x, __CT_d | __CT_x, __CT_d | __CT_x,
                               __CT_d | __CT_x, /* 0x30..0x33 */
                               __CT_d | __CT_x, __CT_d | __CT_x, __CT_d | __CT_x,
                               __CT_d | __CT_x, /* 0x34..0x37 */
                               __CT_d | __CT_x, __CT_d | __CT_x, __CT_p, __CT_p, /* 0x38..0x3B */
                               __CT_p, __CT_p, __CT_p, __CT_p, /* 0x3C..0x3F */
                               __CT_p, __CT_u | __CT_x, __CT_u | __CT_x, __CT_u | __CT_x, /* 0x40..0x43 */
                               __CT_u | __CT_x, __CT_u | __CT_x, __CT_u | __CT_x, __CT_u, /* 0x44..0x47 */
                               __CT_u, __CT_u, __CT_u, __CT_u, /* 0x48..0x4B */
                               __CT_u, __CT_u, __CT_u, __CT_u, /* 0x4C..0x4F */
                               __CT_u, __CT_u, __CT_u, __CT_u, /* 0x50..0x53 */
                               __CT_u, __CT_u, __CT_u, __CT_u, /* 0x54..0x57 */
                               __CT_u, __CT_u, __CT_u, __CT_p, /* 0x58..0x5B */
                               __CT_p, __CT_p, __CT_p, __CT_p, /* 0x5C..0x5F */
                               __CT_p, __CT_l | __CT_x, __CT_l | __CT_x, __CT_l | __CT_x, /* 0x60..0x63 */
                               __CT_l | __CT_x, __CT_l | __CT_x, __CT_l | __CT_x, __CT_l, /* 0x64..0x67 */
                               __CT_l, __CT_l, __CT_l, __CT_l, /* 0x68..0x6B */
                               __CT_l, __CT_l, __CT_l, __CT_l, /* 0x6C..0x6F */
                               __CT_l, __CT_l, __CT_l, __CT_l, /* 0x70..0x73 */
                               __CT_l, __CT_l, __CT_l, __CT_l, /* 0x74..0x77 */
                               __CT_l, __CT_l, __CT_l, __CT_p, /* 0x78..0x7B */
                               __CT_p, __CT_p, __CT_p, __CT_c /* 0x7C..0x7F */
                             };

int toupper(int c) {
    return (islower(c) ? (c ^ 0x20) : (c));
}

int tolower(int c) {
    return (isupper(c) ? (c ^ 0x20) : (c));
}
