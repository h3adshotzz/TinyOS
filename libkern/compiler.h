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
/**
 *
 * Source: Trusted-Firmware-A
 *
 * Copyright (C) 2016-2019, Arm Limited and Contributors. All rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __LIBKERN_COMPILER_H__
#define __LIBKERN_COMPILER_H__

/*
 * For those constants to be shared between C and other sources, apply a 'U',
 * 'UL', 'ULL', 'L' or 'LL' suffix to the argument only in C, to avoid
 * undefined or unintended behaviour.
 *
 * The GNU assembler and linker do not support these suffixes (it causes the
 * build process to fail) therefore the suffix is omitted when used in linker
 * scripts and assembler files.
*/
#if defined(__ASSEMBLER__)
# define   U(_x)	(_x)
# define  UL(_x)	(_x)
# define ULL(_x)	(_x)
# define   L(_x)	(_x)
# define  LL(_x)	(_x)
#else
# define  U_(_x)	(_x##U)
# define   U(_x)	U_(_x)
# define  UL_(_x)	(_x##UL)
# define  UL(_x)	UL_(_x)
# define  ULL_(_x)	(_x##ULL)
# define  ULL(_x)	ULL_(_x)
# define  L_(_x)	(_x##L)
# define  L(_x)	L_(_x)
# define  LL_(_x)	(_x##LL)
# define  LL(_x)	LL_(_x)
#endif /* __assembler__ */

#define BIT_32(nr)		((int) 1 << (nr))
#define BIT_64(nr)		((long) 1 << (nr))


#endif /* __libkern_compiler_h__ */
