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
 * Name:	vm_page.c
 * Desc:	Virtual Memory Page Allocator/Manager.
*/

#include <tinylibc/string.h>
#include <kern/vm/vm_page.h>

typedef queue_head_t		vm_page_queue_head_t;

/* page queues */
static vm_page_queue_head_t vm_page_queue;			/* allocated pages */
static vm_page_queue_head_t vm_page_free_queue;		/* free pages */

static uint64_t				vm_page_idx;

/* page queue region bounds */
static vm_address_t			vm_page_queue_region_lower_bound __attribute__((section(".vm")));
static vm_address_t			vm_page_queue_region_upper_bound;
static vm_address_t			vm_page_queue_region_cursor;


static void
__vm_page_queue_region_inc ()
{
	vm_page_queue_region_cursor += sizeof (vm_page_t);
}

/**
 * Internal page initialiser. Creates a vm_page_t at a given physical address and
 * adds it to the page free queue.
*/
static int
__vm_page_create (phys_addr_t paddr)
{
	vm_page_t		*page;

	/* check that the page queue region hasn't been exceeded */
	if (vm_page_queue_region_cursor >= vm_page_queue_region_upper_bound) {
		vm_page_log ("error: page queue region exceeded upper bounds\n");
		return 1;
	}

	/* get the location in the .vm section to create the page structure */
	page = vm_page_queue_region_cursor;

	kprintf("");

	/* create the page structure */
	page->paddr = paddr;
	page->flags = 0;

	page->idx = vm_page_idx;
	vm_page_idx++;

	/* add it to the tail-end of the page free queue */
	enqueue_tail (&vm_page_free_queue, (queue_entry_t *) page);

	__vm_page_queue_region_inc ();

	return 0;
}

void
vm_page_bootstrap (phys_addr_t start, phys_addr_t end)
{
	phys_size_t		psize;
	phys_addr_t		pcursor;
	uint64_t		page_count;
	int				res;

	vm_page_log ("starting page bootstrap\n");
	vm_page_log ("vm_page_queue: 0x%llx\n", &vm_page_queue);
	vm_page_log ("vm_page_free_queue: 0x%llx\n", &vm_page_free_queue);

	/* set the initial index */
	vm_page_idx = 0;

	/* calculate the actualy physical memory size, and total number of pages */
	psize = end - start;
	page_count = psize / VM_PAGE_SIZE;

	vm_page_log ("page count: %d, start: 0x%llx\n", page_count, start);
	vm_page_log ("size required: %d bytes, %d kilobytes\n",
		page_count * sizeof(vm_page_t), (page_count * sizeof(vm_page_t)) / 1024);

	/* initialise the page queues */
	queue_init (&vm_page_queue);
	queue_init (&vm_page_free_queue);

	/* initialise the page queue region */
	vm_page_queue_region_upper_bound = &vm_page_queue_region_lower_bound + (page_count * sizeof (vm_page_t));
	vm_page_queue_region_cursor = &vm_page_queue_region_lower_bound;

	vm_page_log ("vm_page_queue_region: 0x%llx->0x%llx\n",
		&vm_page_queue_region_lower_bound, vm_page_queue_region_upper_bound);
	vm_page_log ("vm_page_queue_region_cursor: 0x%llx\n", vm_page_queue_region_cursor);

	pcursor = start;
	for (unsigned int i = 0; i < page_count; i++) {
		if (__vm_page_create (pcursor) != 0)
			break;
		pcursor += VM_PAGE_SIZE;
	}
	vm_page_log ("created %d pages\n", page_count);

	vm_page_t *page = (vm_page_t *) vm_page_queue_region_lower_bound;
	queue_element (&page, vm_page_t, pageq);
	vm_page_log ("page[%d]: 0x%llx\n", page->idx, page->paddr);

//	queue_foreach_element (page, &vm_page_free_queue, pageq) {
//		vm_page_log ("page[%d]: 0x%llx\n", page->idx, page->paddr);
//	}
}