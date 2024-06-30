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
 * Name:	zalloc.h
 * Desc:	Zone allocator. The kernel can use zones to manage data structures
 * 			memory dynamically, with zones being created for each type of data
 * 			structure to be managed. Based on Mach's zalloc.h
*/

#ifndef __KERN_ZALLOC_H__
#define __KERN_ZALLOC_H__

#include <tinylibc/stdint.h>

#include <libkern/types.h>
#include <libkern/list.h>

#include <kern/kprintf.h>
#include <kern/vm/vm_types.h>

/* Interface logger */
#define zalloc_log(fmt, ...)		interface_log("zalloc", fmt, ##__VA_ARGS__)

#define ZONE_NULL					NULL

/**
 * The Zone Allocator
 * 
 * Zone "descriptors" are created and stored within the zone_array. They contain
 * information regarding the whole zone, such as the number of in-use and free
 * elements, the list of free and used allocation metadata's, number of virtual
 * memory pages used for the zone, name, index, etc.
 * 
 * Pages are allocated for the requested size (number of elements) for the zone,
 * these pages are then filled with zone_alloc_metadata structs. These structs:
 * 
 * 		1) Contain only the list_node_t
 * 		2) Are directly followed by the element data
 * 
 * In the zone structure, the two lists contain these metadata structs. When an
 * allocation is made, the first element from the free list is takne, mode into
 * the used list, and the address of the metadata + the element size is returned.
 * 
*/
typedef struct zone {

	integer_t	count;			/* Number of in-use elements */
	integer_t	count_free;		/* Number of free elements */

	vm_size_t	size;			/* Current zone size */
	vm_size_t	max_size;		/* Maximum zone size */
	vm_size_t	elem_size;		/* Zone element size */

	integer_t	page_count;		/* Number of pages used by this zone */

	list_t		free_elems;		/* List of free elements */
	list_t		used_elems;		/* List of used elements */

	integer_t	index;			/* Zone index */
	const char	*name;			/* Zone name */

	uint32_t	

#define ZONE_STATE_UNUSED		(0x0)
#define ZONE_STATE_USED			(0x1)
	/* boolean_t */	state 		:1,			/* Current state (used/not used) */

	/* future    */ _reserved	:30;		/* Reserved for future use as flags */
} zone_t;

/**
 * The pages allocated for the zone are filled with these structs, with enough
 * space for the element data type after each of them. 
*/
typedef struct zone_alloc_metadata {
	list_node_t	alloc;
} zone_alloc_metadata_t;

extern kern_return_t zone_init();
extern zone_t *zone_create(vm_size_t size, vm_size_t max, const char *name);

extern void *zalloc(zone_t *zone);
extern void zfree(zone_t *zone, vm_address_t addr);

extern void zone_dump(zone_t *zone);

#endif /* __kern_zalloc_h__ */
