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

#ifndef __TLIBC_PRIV_TYPES_H__
#define __TLIBC_PRIV_TYPES_H__

////////////////////////////////////////////////////////

#define __TINYLIBC_NULL             (0)
#define NULL                        __TINYLIBC_NULL

#define true                1
#define false               0

////////////////////////////////////////////////////////

/* unsigned integers */
typedef unsigned char                 __uint8_t;
typedef unsigned short                __uint16_t;
typedef unsigned int                  __uint32_t;
typedef unsigned long long            __uint64_t;

typedef unsigned long                 __u_long;

/* signed integers */
typedef char                          __int8_t;
typedef short                         __int16_t;
typedef int                           __int32_t;
typedef long long                     __int64_t;

/* maximums */
typedef long unsigned int             __uintmax_t;
typedef long int                      __intmax_t;

/* pointers */
typedef long                          __intptr_t;
typedef unsigned long                 __uintptr_t;

/* boolean */
typedef int                             __boolean;

/**
 * Name:    offset_of
*/
#if defined(__GNUC__)
#define __offsetof(type, field)     __builtin_offsetof(type, field)
#else
#define __offsetof(type, field)     ((size_t)(&((type *)0)->field))
#endif /* __gnuc__ */

#endif /* __tlibc_priv_types_h__ */