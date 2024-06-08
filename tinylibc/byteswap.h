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

#ifndef __BYTESWAP_H__
#define __BYTESWAP_H__

#include <tinylibc/_byteswap.h>

/* 16-bit argument swap */
#define bswap_16(x)         __bswap_16 (x)

/* 32-bit argument swap */
#define bswap_32(x)         __bswap_32 (x)

/* 64-bit argument swap */
#define bswap_64(x)         __bswap_64 (x)

#endif /* __byteswap_h__ */