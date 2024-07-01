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
 * Name:	vm_map.h
 * Desc:	Virtual memory mapping manager.
*/

#ifndef __KERN_VM_MAP_H__
#define __KERN_VM_MAP_H__

#include <tinylibc/stdint.h>

#include <libkern/list.h>

#include <kern/vm/vm_types.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

/* interface logger */
#define vm_map_log(fmt, ...)		interface_log("vm_map", fmt, ##__VA_ARGS__)

/* Align an address to a 4-byte boundary */
#define VM_ALIGN_ADDR(_addr)		((_addr + (4 - 1)) & -4)

#define VM_MAP_ENTRY_SIZE			(sizeof(vm_map_entry_t))

#define VM_NULL						(0x0)
#define VM_FALSE					(0x0)
#define VM_TRUE						(0x1)

/* Flags for allocating virtual memory */
#define VM_ALLOC_GUARD_FIRST		(0x01)	/* guard page before allocation */
#define VM_ALLOC_GUARD_LAST			(0x02)	/* guard page after allocation */
#define VM_ALLOC_KERNEL_CODE		(0x04)	/* kernel code */

#define VM_MAP_ENTRY_GUARD_PAGE		(0x01)

/**
 * Describes a virtual memory mapping entry. These correspond with physical
 * translation table mappings, and are used to track what virtual memory space
 * has been allocated for a particular vm_map_t.
*/
typedef struct vm_map_entry {

	/* Base and size of the virtual memory map entry */
	vm_address_t	base;
	vm_size_t		size;

	/* Flags */
	uint32_t		guard_page	:1,
					kernel_code	:1,
					__unused_bits:30;

	/* List of entries */
	list_node_t		siblings;

} vm_map_entry_t;

/**
 * Virtual memory map for a task.
*/
typedef struct vm_map {
	uint64_t		timestamp;

	/* Pointer to the pmap for this vm_map */
	pmap_t			*pmap;

	/* Virtual address region available for this map */
	vm_address_t	min;
	vm_address_t	max;

	/* Current allocated size */
	vm_size_t		size;

	/* Flags */
	unsigned int	lock:1,
					__unused_bits:31;

	uint32_t		nentries;
	list_t			entries;
} vm_map_t;

/* virtual memory maps */
extern void vm_map_create 	(vm_map_t *map, pmap_t *pmap, vm_address_t min,
								vm_address_t max);
extern void vm_map_unlock 	(vm_map_t *map);
extern void vm_map_lock 	(vm_map_t *map);

extern vm_address_t vm_map_alloc (vm_map_t *map, vm_size_t size,
								vm_flags_t flags);

/* virtual memory map entries */
extern void vm_map_entry_create (vm_map_t *map, vm_address_t base,
								vm_size_t size, vm_flags_t flags);

vm_map_t *vm_map_create_new (pmap_t *pmap, vm_address_t min, vm_address_t max);

// maybe move?
extern vm_map_t *vm_get_kernel_map();

#endif /* __kern_vm_map_h__ */