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
 *	Name:	vm_types.h
 *	Desc:	Virtual memory type definitions.
 */

#ifndef __KERN_VM_TYPES_H__
#define __KERN_VM_TYPES_H_

#include <tinylibc/stdint.h>

typedef uint64_t	vm_address_t;		/* Virtual memory address */
typedef uint64_t	vm_offset_t;		/* Virtual memory offset */
typedef uint64_t	vm_size_t;			/* Virtual memory size */

typedef int			vm_map_type_t;		/* Mapping region type */
typedef int			vm_prot_t;			/* Mapping protection */

#endif
