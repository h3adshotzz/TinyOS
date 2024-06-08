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

#include <kern/vm/vm_page.h>
#include <kern/vm/vm_map.h>
#include <kern/vm/pmap.h>

#include <libkern/assert.h>
#include <tinylibc/string.h>

/*******************************************************************************
 * Name:	vm_map_entry_create
 * Desc:	Create a new entry within a vm_map for the given base address and
 * 			size. This does not allocate the 'size' of memory at 'base', it is
 * 			expected that this has already been done.
*******************************************************************************/

void vm_map_entry_create (vm_map_t *map, vm_address_t base, vm_size_t size)
{
	vm_map_entry_t *entry;

	vm_map_lock(map);

	entry = (vm_map_entry_t *) ((map + sizeof(vm_map_t)) +
		(map->nentries * VM_MAP_ENTRY_SIZE));
	memset(entry, '\0', sizeof(vm_map_entry_t));

	entry->base = base;
	entry->size = size;

	map->nentries += 1;
	map->size += size;

	vm_map_unlock(map);
}

/*******************************************************************************
 * Locking for vm_map_t
 * 
 * TODO: These don't have any actual functionality yet. Need to research how
 * locking is used in the kernel to understand what we actually need to do here.
*******************************************************************************/

void vm_map_lock (vm_map_t *map)
{
	map->lock = 1;
}

void vm_map_unlock (vm_map_t *map)
{
	map->lock = 0;
}

/*******************************************************************************
 * Name:	__vm_map_init
 * Desc:	Initialise an empty virtual memory map with a given address range
 * 			and pmap_t.
*******************************************************************************/

static void __vm_map_init (vm_map_t *map, pmap_t *pmap, vm_address_t min,
						vm_address_t max)
{
	map->timestamp = 0;		/* TODO */
	map->pmap = pmap;
	map->min = min;
	map->max = max;
	map->size = 0;

	/* TODO: implement locking */
	map->lock = 1;

	/**
	 * Map entries should follow the map structure in memory. We'll set the
	 * entries pointer to directly after the map.
	*/
	map->nentries = 0;
}

/*******************************************************************************
 * Name:	vm_map_create
 * Desc:	Create a new virtual memory map for the given address range and pmap
*******************************************************************************/

void vm_map_create (vm_map_t *map, pmap_t *pmap, vm_address_t min,
						vm_address_t max)
{
	__vm_map_init(map, pmap, min, max);
	vm_map_unlock(map);

	/* TODO: check that `map` is on a page boundary */

	vm_map_log("created new vm_map at 0x%lx for virtual address range: 0x%lx-0x%lx\n",
		map, min, max);
}

/* todo */
vm_map_t *vm_map_create_new (pmap_t *pmap, vm_address_t min, vm_address_t max)
{
	vm_map_log("creating a new vm_map for virtual region: 0x%lx - 0x%lx\n",
		min, max);

	phys_addr_t paddr;
	vm_map_entry_t entry;
	vm_map_t map;
	tt_table_t *ttep;

	map.timestamp = 0;		/* TODO */
	map.pmap = pmap;
	map.min = min;
	map.max = max;
	map.size = 0;
	map.lock = 1;
	map.nentries = 0;

	entry.base = min;
	entry.size = VM_PAGE_SIZE;

	paddr = vm_page_alloc();

	ttep = pmap->ttep;
	pmap_tt_create_tte(ttep, paddr, min, VM_PAGE_SIZE);
}

/*******************************************************************************
 * Name:	vm_map_alloc
 * Desc:	Allocate virtual memory for a given size within the provided vm_map,
 * 			and create corresponding entries in the mmu translation tables so
 * 			the allocation is immediately accessible.
 * 
 * 			This allocator makes sequential vm_map_entry's, meaning each entry
 * 			is placed one-after-the-other.
*******************************************************************************/

vm_address_t vm_map_alloc (vm_map_t *map, vm_size_t size)
{
	vm_map_entry_t *last_entry;
	uint32_t page_count;
	vm_address_t vbase;
	pmap_t *pmap;

	/* use the end of the last map entry as the base for the next */
	last_entry = (vm_map_entry_t *) (map + (VM_MAP_ENTRY_SIZE * map->nentries));
	page_count = (size < VM_PAGE_SIZE) ? 1 : size / VM_PAGE_SIZE;

	/* work out the base address for the allocation */
	vbase = (vm_address_t) (last_entry->base + last_entry->size);

	/**
	 * reserve enough physical pages and create entries in the translation table
	 * pointed to by the map->pmap.
	*/
	pmap = &map->pmap;
	for (int i = 0; i < page_count; i++) {
		phys_addr_t page_addr;

		page_addr = vm_page_alloc();
		pmap_tt_create_tte(&pmap->tte, page_addr, vbase, VM_PAGE_SIZE);

		vbase += VM_PAGE_SIZE;
	}

	/* create the vm_map_entry */
	vm_map_entry_create(map, vbase, page_count * VM_PAGE_SIZE);
	return vbase;
}

/*******************************************************************************
 * Name:	vm_map_alloc_at_address
 * Desc:	Allocate virtual memory of a given size within the provided vm_map,
 * 			and create the corresponding entries in the mmu translation tables
 * 			from a provided virtual base address.
 * 
 * 			Unlike vm_map_alloc, this alloc takes a virtual address to map `size`
 * 			bytes to. 
*******************************************************************************/

int vm_map_alloc_at_address (vm_map_t *map, vm_size_t size, vm_address_t base)
{
	/**
	 * this allocator may be used when creating the virtual address space of a
	 * new process, where the __DATA, __TEXT, heap and stack are best not placed
	 * directly next to eachother.
	*/
	return 0;
}