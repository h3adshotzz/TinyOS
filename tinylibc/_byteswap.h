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

#ifndef __TLIBC_PRIV_BYTESWAP_H__
#define __TLIBC_PRIV_BYTESWAP_H__

#include <tinylibc/_types.h>

#define __bswap_16_internal(x)      ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define __bswap_32_internal(x)      ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)	\
                                    | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))
#define __bswap_64_internal(x)      ((((x) & 0xff00000000000000ull) >> 56)	\
                                    | (((x) & 0x00ff000000000000ull) >> 40)	\
                                    | (((x) & 0x0000ff0000000000ull) >> 24)	\
                                    | (((x) & 0x000000ff00000000ull) >> 8)	\
                                    | (((x) & 0x00000000ff000000ull) << 8)	\
                                    | (((x) & 0x0000000000ff0000ull) << 24)	\
                                    | (((x) & 0x000000000000ff00ull) << 40)	\
                                    | (((x) & 0x00000000000000ffull) << 56))


static __inline __uint16_t
__bswap_16 (__uint16_t __bs)
{
    return __bswap_16_internal (__bs);
}

static __inline __uint32_t
__bswap_32 (__uint32_t __bs)
{
    return __bswap_32_internal (__bs);
}

static __inline __uint32_t
__bswap_64 (__uint32_t __bs)
{
    return __bswap_64_internal (__bs);
}

#endif /* __tlibc_priv_byteswap_h__ */