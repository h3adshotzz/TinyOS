//===----------------------------------------------------------------------===//
//
//                            tinyOS - tinyLibC
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
//  Copyright (C) 2023, Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <tinylibc/_types.h>

typedef __uint32_t			size_t;

/* va_list */
#define va_list				__builtin_va_list
#define va_start			__builtin_va_start
#define va_arg				__builtin_va_arg
#define va_end				__builtin_va_end

/* offsetof */
#define offsetof(t, d)		__builtin_offsetof(t, d)

#endif /* __stddef_h__ */