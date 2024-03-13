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
 * 	Name:	kprintf.h
 * 	Desc:	Kernel printf.
 */

#ifndef __KERN_KPRINTF_H__
#define __KERN_KPRINTF_H__

#include <libkern/types.h>
#include <tinylibc/stddef.h>

int kprintf (const char *fmt, ...);
int vprintf (const char *fmt, va_list ap);

#endif /* __kern_kprintf_h__ */