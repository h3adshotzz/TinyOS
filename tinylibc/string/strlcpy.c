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

size_t
strlcpy (char *dest, const char *src, size_t size)
{
    size_t srclen, len;

    if (size) {
        srclen = strlen (src);
        len = (srclen >= size) ? size - 1 : srclen;

        memcpy (dest, src, len);
        dest[len] = '\0';
        return len;
    }
    return 0;
}