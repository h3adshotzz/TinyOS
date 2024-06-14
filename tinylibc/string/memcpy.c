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
memcpy (void *dest, const void *src, size_t count)
{
    unsigned long *dl = (unsigned long *) dest, *sl = (unsigned long *) src;
    char *d8, *s8;

    if (src == dest)
        return dest;

    /* while all data is aligned (common case), copy a word at a time */
    if ( (((uint64_t) dest | (uint64_t) src) & (sizeof (*dl) - 1)) == 0) {
        while (count >= sizeof (*dl)) {
            *dl++ = *sl++;
            count -= sizeof (*dl);
        }
    }

    /* copy the rest one byte at a time */
    d8 = (char *) dl;
    s8 = (char *) sl;
    while (count--)
        *d8++ = *s8++;
    return dest;
}