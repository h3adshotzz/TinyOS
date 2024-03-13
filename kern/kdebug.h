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
 * 	Name:	kdebug.h
 * 	Desc:	Kernel debug interface.
 */


#ifndef __KERNEL_DEBUG_H__
#define __KERNEL_DEBUG_H__

#include <libkern/types.h>


/* Kernel Early Logging APIs */
void kernel_debug_early_log_init ();
void kernel_debug_early_log ();

/* Kernel Debugging macros */
#define KERNEL_DEBUG_HALT()                             \
    do {                                                \
        kprintf ("\n\nKERNEL_DEBUG: halting...\n");     \
        asm volatile ("brk #1");                        \
    } while (0);

#endif /* __kernel_debug_h__ */