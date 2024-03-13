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

#ifndef __STDINT_H__
#define __STDINT_H__

#include <tinylibc/_types.h>
#include <tinylibc/limits.h>
#include <tinylibc/stddef.h>

/* define user integer types */
typedef __uint8_t           uint8_t;
typedef __uint16_t          uint16_t;
typedef __uint32_t          uint32_t;
typedef __uint64_t          uint64_t;

typedef __u_long            u_long;

typedef __int8_t            int8_t;
typedef __int16_t           int16_t;
typedef __int32_t           int32_t;
typedef __int64_t           int64_t;

typedef __uintmax_t         uintmax_t;
typedef __intmax_t          intmax_t;

typedef __uintptr_t         uintptr_t;
typedef __intptr_t          intptr_t;

/* booleans */
typedef __boolean           bool;

/* limits of integral types */

/* Minimum of signed integral types.  */
#define INT8_MIN      (-128)
#define INT16_MIN     (-32767-1)
#define INT32_MIN     (-2147483647-1)
#define INT64_MIN     (-__INT64_C(9223372036854775807)-1)

/* Maximum of signed integral types.  */
#define INT8_MAX      (127)
#define INT16_MAX     (32767)
#define INT32_MAX     (2147483647)
#define INT64_MAX     (__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
#define UINT8_MAX     (255)
#define UINT16_MAX    (65535)
#define UINT32_MAX    (4294967295U)
#define UINT64_MAX    (__UINT64_C(18446744073709551615))


/* Signed */
#define INT8_C(c)     c
#define INT16_C(c)    c
#define INT32_C(c)    c
#define INT64_C(c)    c ## L

/* Unsigned */
#define UINT8_C(c)    c
#define UINT16_C(C)   c
#define UINT32_C(c)   c ## U
#define UINT64_C(c)   c ## UL

/* Maximal type */
#define INTMAX_C(c)   c ## L
#define UINTMAX_C(c)  c ## UL

#endif /* __stdint_h__ */
