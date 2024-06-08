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

int
strncmp (const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    do {
        if (*s1 != *s2++)
            return (*(unsigned char *) s1 - *(unsigned char *) --s2);
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}