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

#include <kern/vm/vm_types.h>
#include <kern/vm/vm_page.h>
#include <kern/vm/vm_map.h>
#include <kern/mm/zalloc.h>

#include <libkern/bitmap.h>
#include <libkern/panic.h>

unsigned int	num_zones_used;

#define MAX_NUM_ZONES	12
static zone_t	zone_array[MAX_NUM_ZONES];

static void zone_dump_all()
{
	zalloc_log("dumping '%d' zones:\n", num_zones_used);
	for (int i = 0; i < MAX_NUM_ZONES; i++) {
		zone_t *tmp = &(zone_array[i]);
		if (tmp->state == ZONE_STATE_UNUSED)
			continue;

		zone_dump(tmp);
	}
}

void zalloc_tests()
{
	zalloc_log("starting zone allocator tests:\n");

	kprintf("zone_alloc_metadata size: 0x%x\n", sizeof(struct zone_alloc_metadata));

	struct element_test {
		uint64_t val_1;
		uint64_t val_2;
	};

	zone_t *test_zone = zone_create(
		sizeof(struct element_test),
		10 * sizeof(struct element_test),
		"test_element"
	);
	zone_t *test_zone_2 = zone_create(
		sizeof(struct element_test),
		10 * sizeof(struct element_test),
		"test_element2"
	);

	// dump the zone first
	zone_dump_all();

	// allocate three elements
	struct element_test *elem_1 = zalloc(test_zone);
	struct element_test *elem_2 = zalloc(test_zone);
	struct element_test *elem_3 = zalloc(test_zone);

	// try writing to the last allocated element, and print its info
	elem_3->val_1 = 55;
	elem_3->val_2 = 105;
	kprintf("zalloc elem3: ptr: 0x%lx, val1: %d, val2: %d\n",
		elem_3, elem_3->val_1, elem_3->val_2);

	// dump the zones again
	zone_dump_all();

	// be awkward and free the middle one
	zfree(test_zone, elem_2);

	// dump the zones again
	zone_dump_all();

	// try and free a zone that is already free
//	kprintf ("double free'ing zone element: 0x%lx\n", elem_2);
//	zfree(test_zone, elem_2);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * zone_debug_dump
 * 
 * Dump the contents of a specified zone
*/
void zone_dump(zone_t *zone)
{
	struct zone_alloc_metadata *meta;
	vm_size_t msize;

	msize = sizeof(struct zone_alloc_metadata);

	kprintf("zone[%d]: '%s', size: %d\n", zone->index, zone->name, zone->max_size);
	kprintf("  free: '%d':\n", zone->count_free);
	list_for_each_entry(meta, &zone->free_elems, alloc) {
		vm_address_t addr = meta;
		kprintf("    meta: 0x%lx | element: 0x%lx\n", meta, addr + msize);
	}

	kprintf("  alloc: '%d':\n", zone->count);
	list_for_each_entry(meta, &zone->used_elems, alloc) {
		vm_address_t addr = meta;
		kprintf("    meta: 0x%lx | element: 0x%lx\n", meta, addr + msize);
	}
}

/**
 * zone_init
 * 
 * Initialise the zone_array with empty zone_t's with their states set to
 * ZONE_STATE_UNUSED. When a zone is created, it will go from 0..N and use the
 * first unused zone structure, so they need to be initialised.
 */
kern_return_t zone_init()
{
	/**
	 * we track the number of used zones, although there isn't much use for this
	 * tracking yet.
	*/
	num_zones_used = 0;

	/* init each zone */
	for (int i = 0; i < MAX_NUM_ZONES; i++) {
		zone_t *zone = &(zone_array[i]);

		/* memset each zone, and only set it's 'state' flag */
		memset(zone, '\0', sizeof(zone_t));
		zone->state = ZONE_STATE_UNUSED;
	}

	return KERN_RETURN_SUCCESS;
}

/**
 * zone_create
 * 
 * Creates a new zone for the specified data structure size, and allocates enough
 * pages for the desired zone size, plus metadata structures.
*/
zone_t *zone_create(vm_size_t size, vm_size_t max, const char *name)
{
	vm_address_t	zone_page_base;
	vm_size_t		zone_block_size;
	zone_t			*zone;
	int				zidx;

	zalloc_log("creating zone '%s' for alloc size '%d', and max size '%d'\n",
		name, size, max);

	/* find the first available zone descriptor */
	for (zidx = 0; zidx < MAX_NUM_ZONES; zidx++) {
		zone = &(zone_array[zidx]);
		if (zone->state == ZONE_STATE_UNUSED)
			break;

		/* if one wasn't found, reset the zone to ZONE_NULL */
		zone = ZONE_NULL;
	}

	/* ensure that a zone was found */
	if (zone == ZONE_NULL) {
		panic("failed to allocate a zone for '%s': no available zones\n", name);
		return ZONE_NULL;
	}

	/* ensure the element size is valid */
	if (size == 0) {
		panic("failed to allocate a zoen for '%s': invalid element size: %d\n",
			size);
		return ZONE_NULL;
	}

	/* ensure the maximum zone size is valid */
	if (max == 0) {
		panic("failed to allocate a zone for '%s': invalid max zone size: %d\n",
			max);
		return ZONE_NULL;
	}

	zone->elem_size = size;
	zone->count_free = (max / zone->elem_size);
	zone->count = 0;

	/**
	 * the maximum zone size is calculated without taking into account the zone
	 * element metadata, it only represents how much pure data is contained in
	 * the zone.
	*/
	zone->max_size = zone->count_free * zone->elem_size;
	zone->size = 0;

	/**
	 * when calculating the number of pages, we do need to take into account the
	 * zone metadata structures. this is the number of pages we need to request
	 * from the vm allocator.
	 * 
	 * If the total required size, divided by the page size comes to less than
	 * one page, use one page as a minimum.
	*/
	zone->page_count = (zone->max_size + 
		(max * sizeof(struct zone_alloc_metadata))) / VM_PAGE_SIZE;
	if (zone->page_count == 0)
		zone->page_count = 1;

	/* initialise the free and used lists */
	INIT_LIST_HEAD(&zone->free_elems);
	INIT_LIST_HEAD(&zone->used_elems);

	zone->index = zidx;
	zone->name = name;

	/* allocate enough pages for this zone */
	/**
	 * note, because i'm lazy and can't be bothered to fix the issue with
	 * vm_map_alloc yet, this is how we make sure the assigned address is actually
	 * valid.
	*/
	while (vm_is_address_valid(zone_page_base = vm_map_alloc(vm_get_kernel_map(),
		zone->page_count * VM_PAGE_SIZE)) != KERN_RETURN_SUCCESS);
	zalloc_log("zone: '%s': zone_page_base: 0x%lx\n", name, zone_page_base);

	/**
	 * Each element in the zone has a zone_alloc_metadata structure containing
	 * a list node. This is how free and used blocks are tracked. Before any
	 * allocations can be made, these structures need to be created and added
	 * to the zone free list.
	*/
	zone_block_size = zone->elem_size + sizeof(struct zone_alloc_metadata);
	for (int i = 0; i < zone->count_free; i++) {
		struct zone_alloc_metadata *meta = (struct zone_alloc_metadata *)
			(zone_page_base + (i * zone_block_size));
		list_add_tail(&meta->alloc, &zone->free_elems);
	}

	/* set the zone state, and return it */
	zone->state = ZONE_STATE_USED;
	return zone;
}

/**
 * zalloc
 * 
 * Allocate a new element within a specified zone and return the address.
*/
void *zalloc(zone_t *zone)
{
	struct zone_alloc_metadata	*meta;
	vm_address_t				addr;

	/**
	 * this is a simple process: take the first entry within the freelist, move
	 * it to the used list, update the counters and return the address of the
	 * element (exactly after the zone metadata).
	*/
	meta = list_first_entry(&zone->free_elems, struct zone_alloc_metadata, alloc);
	list_move(&meta->alloc, &zone->used_elems);

	zone->count += 1;
	zone->count_free -= 1;

	addr = (meta);
	return (void *) (addr + sizeof(struct zone_alloc_metadata));
}

/**
 * zfree
 * 
 * Free the element at a given address from the specified zone.
*/
void zfree(zone_t *zone, vm_address_t addr)
{
	struct zone_alloc_metadata	*meta;
	vm_address_t				meta_addr;

	/**
	 * calculate the address of the element's metadata struct, and then loop
	 * over the used list until we find it. at that point, we can clear the
	 * elements (not the metadata struct though), and move the metadata into the
	 * free list.
	*/
	meta_addr = addr - sizeof(struct zone_alloc_metadata);

	list_for_each_entry(meta, &zone->used_elems, alloc) {
		if (meta == meta_addr) {
			memset(addr, '\0', zone->elem_size);
			list_move(&meta->alloc, &zone->free_elems);

			zone->count -= 1;
			zone->count_free += 1;

			break;
		}

		if (list_is_last(meta, &zone->used_elems)) {
			panic("failed to free elemtn '0x%lx' from zone '%s': element does not exist in zone\n",
				addr, zone->name);
		}
	}
}
