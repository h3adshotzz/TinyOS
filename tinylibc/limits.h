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

#ifndef __LIMITS_H__
#define __LIMITS_H__

///////////////////////////////////////////////////////
// TODO: move to bits.h with bit manipulation funcs
#define MIN(a, b)           (((a)<(b))?(a):(b))
#define MAX(a, b)           (((a)>(b))?(a):(b))

#define BIT_32(nr)          ((int) 1 << (nr))
#define BIT_64(nr)          ((long) 1 << (nr))

///////////////////////////////////////////////////////

/* bit definitions */
#define CHAR_BIT            8           /* bit length of a char */
#define WORD_BIT            32          /* bit length of a word */
#define LONG_BIT            64          /* bit length of a long */

/* limits */
#define SCHAR_MAX           127         /* max value for a signed char */
#define SCHAR_MIN           (-128)      /* min value for a signed char */

#define UCHAR_MAX           255         /* max value for an unsigned char */
#define CHAR_MAX            127         /* max value for a char */
#define CHAR_MIN            (-128)      /* min value for a char */

#define USHRT_MAX           65535       /* max value for an unsigned short */
#define SHRT_MAX            32767       /* max value for a short */
#define SHRT_MIN            (-32768)    /* min value for a short */

#define UINT_MAX            0xffffffff    /* max value for an unsigned int */
#define INT_MAX             2147483647    /* max value for an int */
#define INT_MIN             (-INT_MAX-1)  /* min value for an int */

#define ULONG_MAX           0xffffffffffffffffUL  /* max value for an unsigned long */
#define LONG_MAX            0x7fffffffffffffffL   /* max value for a long */
#define LONG_MIN            (-LONG_MAX-1)         /* min value for a long */

#define ULLONG_MAX          0xffffffffffffffffULL /* max value for an unsigned long long */
#define LLONG_MAX           0x7fffffffffffffffLL  /* max value for a long long */
#define LLONG_MIN           (-LLONG_MAX-1)        /* min value for a long long */

#endif /* __limits_h__ */