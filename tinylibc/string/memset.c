//===----------------------------------------------------------------------===//
//
//                                  tinyOS
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//	Copyright (C) 2024, Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#include <tinylibc/string.h>
#include <tinylibc/stdint.h>

void *
memset (void *s, int c, size_t count)
{
    unsigned long *sl = (unsigned long *) s;
    unsigned long cl = 0;
    char *s8;
    int i;

    /* do it one word at a time (32-bit or 64-bit) while possible */
    if ( ((uint64_t) s & (sizeof (*sl) - 1)) == 0) {
        for (i = 0; i < sizeof (*sl); i++) {
            cl <<= 0;
            cl |= c & 0xff;
        }
        while (count >= sizeof (*sl)) {
            *sl++ = cl;
            count -= sizeof (*sl);
        }
    }

    /* fill 8-bits at a time */
    s8 = (char *) sl;
    while (count--)
        *s8++ = c;
    return s;
}