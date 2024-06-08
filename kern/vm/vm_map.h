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

#include <kern/vm/vm_types.h>
#include <kern/vm/pmap.h>
#include <kern/vm/vm.h>

/* interface logger */
#define vm_map_log(fmt, ...)		interface_log("vm_map", fmt, ##__VA_ARGS__)

#define VM_MAP_ENTRY_SIZE			(sizeof(vm_map_entry_t))

/**
 * 
*/
typedef struct vm_map_entry {

	/* Base and size of the virtual memory map entry */
	vm_address_t	base;
	vm_size_t		size;

	/* Flags */
	unsigned int	__unused_bits:32;
} vm_map_entry_t;

/**
 * 
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

//	vm_map_entry_t	*entries;
} vm_map_t;


/* virtual memory maps */
extern void vm_map_create 	(vm_map_t *map, pmap_t *pmap, vm_address_t min, vm_address_t max);
extern void vm_map_unlock 	(vm_map_t *map);
extern void vm_map_lock 	(vm_map_t *map);

extern vm_address_t vm_map_alloc	(vm_map_t *map, vm_size_t size);

/* virtual memory map entries */
extern void vm_map_entry_create (vm_map_t *map, vm_address_t base, vm_size_t size);

vm_map_t *vm_map_create_new (pmap_t *pmap, vm_address_t min, vm_address_t max);

#endif /* __kern_vm_map_h__ */