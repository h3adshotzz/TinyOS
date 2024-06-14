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
 * 	Name:	assert.h
 * 	Desc:	Kernel assert macro.
 */

#ifndef __KERN_ASSERT_H__
#define __KERN_ASSERT_H__

#include <tinylibc/stdint.h>
#include <libkern/panic.h>
#include <kern/kprintf.h>


#define abort() ((void) panic("failed assertion"))

#define __ASSERT_FILE_NAME  __FILE__

#define assert(e)	\
	((void) ((e) ? ((void) 0) : __assert(#e, __ASSERT_FILE_NAME, __LINE__)))
#define __assert(e, file, line) \
	((void) kprintf ("%s:%d: failed assertion '%s'\n", file, line, e), abort())


// Results
#define ASSERT_FALSE		0
#define ASSERT_TRUE			1

#endif /* __kern_assert_h__ */
